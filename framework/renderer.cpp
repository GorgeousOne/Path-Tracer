// -----------------------------------------------------------------------------
// Copyright  : (C) 2014-2017 Andreas-C. Bernstein
// License    : MIT (see the file LICENSE)
// Maintainer : Andreas-C. Bernstein <andreas.bernstein@uni-weimar.de>
// Stability  : experimental
//
// Renderer
// -----------------------------------------------------------------------------

#include <chrono>
#include "renderer.hpp"

Renderer::Renderer(unsigned w, unsigned h, std::string const& file)
		: width_(w), height_(h), color_buffer_(w * h, Color{0.0, 0.0, 0.0}), filename_(file), ppm_(width_, height_) {}

void Renderer::render() {
	std::size_t const checker_pattern_size = 20;

	for (unsigned y = 0; y < height_; ++y) {
		for (unsigned x = 0; x < width_; ++x) {
			Pixel p(x, y);
			if (((x / checker_pattern_size) % 2) != ((y / checker_pattern_size) % 2)) {
				p.color = Color{0.0f, 1.0f, float(x) / height_};
			} else {
				p.color = Color{1.0f, 0.0f, float(y) / width_};
			}

			write(p);
		}
	}
	ppm_.save(filename_);
}

#define PI 3.14159265f
#define MAX_RAY_DEPTH 2

void Renderer::render(Scene const& scene, Camera const& cam) {
	float fov_radians = cam.fov_x / 180 * PI;
	float img_plane_dist = (width_ / 2.0f) / tan(fov_radians / 2);

	glm::vec3 pixel_width = glm::cross(cam.direction, cam.up);
	glm::vec3 pixel_height {cam.up};
	assert(1.0f == 	glm::length(pixel_width));

	// corner of img_plane relative to camera
	glm::vec3 min_corner =
			img_plane_dist * cam.direction
			- (width_ / 2.0f) * pixel_width
			- (height_ / 2.0f) * pixel_height;

	for (unsigned x = 0; x < width_; ++x) {
		for (unsigned y = 0; y < height_; ++y) {

			// vector for 3D position of 2D pixel relative to camera
			glm::vec3 pixel_pos {
				min_corner
				+ static_cast<float>(x) * pixel_width
				+ static_cast<float>(y) * pixel_height};
			
			glm::vec3 ray_dir{ glm::normalize(pixel_pos) };
			Ray ray {cam.position, ray_dir};
			Pixel pixel {x, y};
			pixel.color = trace(ray, scene, MAX_RAY_DEPTH);
			write(pixel);
		}
	}
	auto start = std::chrono::steady_clock::now();
	ppm_.save(filename_);
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::cout << "save " << filename_ << "\n";
	std::cout << elapsed_seconds.count() << "s save time\n";
}

void Renderer::write(Pixel const& p) {
	// flip pixels, because of opengl glDrawPixels
	size_t buf_pos = (width_ * p.y + p.x);

	if (buf_pos >= color_buffer_.size() || (int) buf_pos < 0) {
		std::cerr << "Fatal Error Renderer::write(Pixel p) : "
		          << "pixel out of ppm_ : "
		          << (int) p.x << "," << (int) p.y
		          << std::endl;
	} else {
		color_buffer_[buf_pos] = p.color;
	}
	ppm_.write(p);
}

Color Renderer::trace(Ray const& ray, Scene const& scene, unsigned ray_depth) const {
	HitPoint closest_hit = get_closest_hit(ray, scene);
	return closest_hit.does_intersect ? shade(closest_hit, scene, ray_depth) : Color {};
}

HitPoint Renderer::get_closest_hit(Ray const& ray, Scene const& scene) const {
	HitPoint closest_hit{};

	for (auto const& it : scene.shapes) {
		HitPoint hit = it.second->intersect(ray);

		if (!hit.does_intersect) {
			continue;
		}
		if (!closest_hit.does_intersect || hit.distance < closest_hit.distance) {
			closest_hit = hit;
		}
	}
	return closest_hit;
}

HitPoint Renderer::find_light_block(Ray const& light_ray, float range, Scene const& scene) const {
	for (auto const& it : scene.shapes) {
		HitPoint hit = it.second->intersect(light_ray);

		if (hit.does_intersect && hit.distance <= range) {
			return hit;
		}
	}
	return HitPoint {};
}

Color Renderer::shade(HitPoint const& hit_point, Scene const& scene, unsigned ray_depth) const {
	Color shaded_color {0, 0, 0};
//	shaded_color += normal_color(hit_point);
	shaded_color += phong_color(hit_point, scene);

	if (hit_point.hit_material->m != 0 && ray_depth > 0) {
		shaded_color += reflection(hit_point, scene, ray_depth);
	}
	return tone_map_color(shaded_color);
}

Color Renderer::phong_color(HitPoint const& hit_point, Scene const& scene) const {
	auto material = hit_point.hit_material;
	Color phong_color = scene.ambient.intensity * material->ka;
	Color specular_light {};

	for (PointLight const& light : scene.lights)  {
		glm::vec3 light_dir = light.position - hit_point.position;
		float distance = glm::length(light_dir);
		light_dir = glm::normalize(light_dir);
		Ray light_ray {hit_point.position, light_dir};

		if (find_light_block(light_ray, distance, scene).does_intersect) {
			continue;
		}
		glm::vec3 normal = hit_point.surface_normal;
		float cos_view_angle = glm::dot(normal, light_dir);

		//back culls specular reflection
		if (cos_view_angle < 0) {
			continue;
		}
		//adds diffuse light
		phong_color += (light.intensity * material->kd * cos_view_angle);

		if (material->m != 0) {
			specular_light += specular_color(hit_point.ray_direction, light_dir, normal, light.intensity, material);
		}
	}
	phong_color *= (1 - material->glossiness);
	phong_color += specular_light * material->glossiness;
	return phong_color;
}

Color Renderer::specular_color(
		glm::vec3 const& viewer_dir,
		glm::vec3 const& light_dir,
		glm::vec3 const& normal,
		Color const& light_intensity,
		std::shared_ptr<Material> material) const {

	glm::vec3 reflection_dir = 2 * glm::dot(normal, light_dir) * normal - light_dir;
	float cos_specular_angle = glm::dot(reflection_dir, viewer_dir * -1.0f);

	if (cos_specular_angle < 0) {
		return Color{};
	}
	float specular_factor = pow(cos_specular_angle, material->m);
	return light_intensity * material->ks * specular_factor;
}

Color Renderer::reflection(HitPoint const& hit_point, Scene const& scene, unsigned ray_depth) const {
	glm::vec3 new_dir = glm::reflect(hit_point.ray_direction, hit_point.surface_normal);
	return trace(Ray{hit_point.position, new_dir}, scene, ray_depth - 1) * hit_point.hit_material->glossiness;
}

Color Renderer::normal_color(HitPoint const& hit_point) const {
	return Color {
			(hit_point.surface_normal.x + 1) / 2,
			(hit_point.surface_normal.y + 1) / 2,
			(hit_point.surface_normal.z + 1) / 2
	};
}

Color& Renderer::tone_map_color(Color& color) const {
	color.r /= color.r + 1;
	color.g /= color.g + 1;
	color.b /= color.b + 1;
	return color;
}

float *Renderer::pixel_buffer() const {
	auto* pixel_data = new float[width_ * height_ * 3];

	for (int i = 0; i < color_buffer_.size(); ++i) {
		Color color = color_buffer_[i];
		pixel_data[i * 3] = color.r;
		pixel_data[i * 3 + 1] = color.g;
		pixel_data[i * 3 + 2] = color.b;
	}
	return pixel_data;
}





