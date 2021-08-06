// -----------------------------------------------------------------------------
// Copyright  : (C) 2014-2017 Andreas-C. Bernstein
// License    : MIT (see the file LICENSE)
// Maintainer : Andreas-C. Bernstein <andreas.bernstein@uni-weimar.de>
// Stability  : experimental
//
// Renderer
// -----------------------------------------------------------------------------

#include <chrono>
#include <iomanip>
#include <thread>
#include "renderer.hpp"

#define PI 3.14159265f
#define NORMAL_COUNT 200

Renderer::Renderer(unsigned w, unsigned h, std::string const& file)
		: width_(w), height_(h), color_buffer_(w * h, Color{0.0, 0.0, 0.0}), filename_(file), ppm_(width_, height_) {

	float turnFraction = (1 + sqrt(5)) / 2;

	for (unsigned i = 0; i < NORMAL_COUNT; ++i) {
		float t = i / (NORMAL_COUNT - 1.0f);
		float pitch = acos(1 - 2*t);
		float yaw = 2*PI * turnFraction * i;

		float x = sin(pitch) * cos(yaw);
		float y = sin(pitch) * sin(yaw);
		float z = cos(pitch);
		normal_sphere_.emplace_back(x, y, z);
	}
	std::cout << normal_sphere_.size() << " rays\n";
}

#define PI 3.14159265f
#define MAX_RAY_DEPTH 2

void Renderer::render(Scene const& scene, Camera const& cam) {
	auto start = std::chrono::steady_clock::now();

	glm::vec4 u = glm::vec4(glm::cross(cam.direction, cam.up), 0);
	glm::vec4 v = glm::vec4(glm::cross({u.x, u.y, u.z}, cam.direction), 0);
	glm::mat4 c {u, v, glm::vec4{-cam.direction, 0}, glm::vec4{cam.position, 1}};

	float img_plane_dist = (width_ / 2.0f) / tan(cam.fov_x / 2);
	float min_x = -(width_ / 2.0f);
	float min_y = -(height_ / 2.0f);

	float percent = 0;
	std::cout << std::fixed;
	std::cout << std::setprecision(2);

	for (unsigned x = 0; x < width_; ++x) {
		for (unsigned y = 0; y < height_; ++y) {
			glm::vec3 ray_dir = glm::normalize(glm::vec3{min_x + x, min_y + y, -img_plane_dist});
			Ray ray = transform_ray({{}, ray_dir}, c);
			Pixel pixel {x, y};
			pixel.color = trace(ray, scene, MAX_RAY_DEPTH, 1);
			write(pixel);
		}
		if (x >= (percent+0.01) * width_) {
			auto end = std::chrono::steady_clock::now();
			std::chrono::duration<double> elapsed_seconds = end-start;
			std::cout << static_cast<int>(100*percent) << "%  " << elapsed_seconds.count() << "s    ~" << elapsed_seconds.count() / percent << "s\n";
			percent += 0.01f;
		}
	}
	ppm_.save(filename_);
	std::cout << "save " << filename_ << "\n";
}

void Renderer::render_threaded(Scene const& scene, Camera const& cam) {
	pixel_index_ = 0;

	auto start = std::chrono::steady_clock::now();

	glm::vec4 u = glm::vec4(glm::cross(cam.direction, cam.up), 0);
	glm::vec4 v = glm::vec4(glm::cross({u.x, u.y, u.z}, cam.direction), 0);
	glm::mat4 c {u, v, glm::vec4{-cam.direction, 0}, glm::vec4{cam.position, 1}};

	float img_plane_dist = (width_ / 2.0f) / tan(cam.fov_x / 2);
	float min_x = -(width_ / 2.0f);
	float min_y = -(height_ / 2.0f);

	float percent = 0;
	std::cout << std::fixed;
	std::cout << std::setprecision(2);

	const size_t thread_count = std::thread::hardware_concurrency();
	std::cout << "using " << thread_count << " threads to render\n";

	std::vector<std::thread> threads;
	threads.resize(thread_count);

	for (std::thread& t : threads) {
		t = std::thread(&Renderer::thread_function, this, scene, c, min_x, min_y, img_plane_dist);
	}
	for (std::thread& t : threads) {
		t.join();
	}
	ppm_.save(filename_);
	std::cout << "save " << filename_ << "\n";
}

void Renderer::thread_function(Scene const& scene, glm::mat4 const& c, float min_x, float min_y, float img_dist) {
	while(true) {
		unsigned current_index = pixel_index_++;
//		++pixel_index_;

		if (current_index >= width_ * height_) {
			return;
		}
		unsigned x = current_index / width_;
		unsigned y = current_index % width_;

		glm::vec3 ray_dir = glm::normalize(glm::vec3{min_x + x, min_y + y, -img_dist});
		Ray ray = transform_ray({{}, ray_dir}, c);
		Pixel pixel{x, y};
		pixel.color = trace(ray, scene, MAX_RAY_DEPTH, 1);
		write(pixel);

		if (x >= (progress + 0.01) * width_) {
			std::cout << static_cast<int>(100 * progress) << "% completed\n";
			progress = progress + 0.01f;
		}
	}
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

Color Renderer::trace(Ray const& ray, Scene const& scene, unsigned ray_depth, unsigned bounce_depth) const {
	HitPoint closest_hit = get_closest_hit(ray, scene);
	return closest_hit.does_intersect ? shade(closest_hit, scene, ray_depth, bounce_depth) : Color {};
}

HitPoint Renderer::get_closest_hit(Ray const& ray, Scene const& scene) const {
	HitPoint closest_hit{};
	HitPoint hit = scene.root->intersect(ray);

	if (!hit.does_intersect) {
		return hit;
	}
	if (!closest_hit.does_intersect || hit.distance < closest_hit.distance) {
		closest_hit = hit;
	}
	return closest_hit;
}

HitPoint Renderer::find_light_block(Ray const& light_ray, float range, Scene const& scene) const {
	HitPoint hit = scene.root->intersect(light_ray);

	if (hit.does_intersect && hit.distance <= range) {
		return hit;
	}
	return HitPoint {};
}

Color Renderer::shade(HitPoint const& hit_point, Scene const& scene, unsigned ray_depth, unsigned bounce_depth) const {
	Color shaded_color {};

	if (bounce_depth > 0) {
		shaded_color += photon_color(hit_point, scene, bounce_depth);
	}else {
		shaded_color += phong_color(hit_point, scene);
	}

	if (hit_point.hit_material->glossiness != 0 && ray_depth > 0) {
		shaded_color += reflection(hit_point, scene, ray_depth);
	}
	return tone_map_color(shaded_color);
}

Color Renderer::phong_color(HitPoint const& hit_point, Scene const& scene) const {
	auto material = hit_point.hit_material;
	Color phong_color{}; //= scene.ambient.intensity * material->ka;
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
		if (material->glossiness != 1) {
			phong_color += (light.intensity * material->kd * cos_view_angle);
		}
		if (material->glossiness != 0) {
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
	return trace(Ray{hit_point.position, new_dir}, scene, ray_depth - 1, 0) * hit_point.hit_material->glossiness;
}

Color Renderer::photon_color(HitPoint const& hit_point, Scene const& scene, unsigned bounce_depth) const {
	Color photon_light{};

	for (Ray const& ray : ray_hemisphere(hit_point.position, hit_point.surface_normal)) {
		photon_light += trace(ray, scene, 0, bounce_depth - 1);
	}
	return photon_light *= hit_point.hit_material->kd * (1 - hit_point.hit_material->glossiness);
}

std::vector<Ray> Renderer::ray_hemisphere(glm::vec3 const& origin, glm::vec3 const& normal) const {
	std::vector<Ray> rays;

	for (glm::vec3 const& dir : normal_sphere_) {
		if (glm::dot(normal, dir) > 0) {
			rays.push_back({origin, dir});
		}
	}
	return rays;
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
