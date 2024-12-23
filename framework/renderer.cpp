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

#define EPSILON 0.001f

Renderer::Renderer(unsigned w, unsigned h, std::string const& file, unsigned aa_steps, unsigned max_ray_bounces) :
		width_(w),
		height_(h),
		color_buffer_(w * h, Color{0.0, 0.0, 0.0}),
		filename_(file), ppm_(width_, height_),
		aa_steps_(aa_steps),
		max_ray_bounces_(max_ray_bounces) {}

#define PI 3.14159265f

void Renderer::render(Scene const& scene) {
	Camera cam = scene.camera;
	float fov_radians = cam.fov_x / 180 * PI;
	float img_plane_dist = (width_ / 2.0f) / tan(fov_radians / 2);

	glm::vec3 u = glm::cross(cam.direction, cam.up);
	glm::vec3 v = glm::cross(u, cam.direction);
	glm::mat4 trans_mat{
			glm::vec4{u, 0},
			glm::vec4{v, 0},
			glm::vec4{-cam.direction, 0},
			glm::vec4{cam.position, 1}
	};
	//gets amount of parallel threads supported by hardware
	size_t core_count = std::thread::hardware_concurrency();
	std::vector<std::thread> threads;
	threads.resize(core_count);

	auto start = std::chrono::steady_clock::now();
	pixel_index_ = 0;

	//starts parallel threads all doing the same task
	for (std::thread& t : threads) {
		t = std::thread(&Renderer::thread_function, this, scene, img_plane_dist, trans_mat);
	}
	//lets main thread wait until all parallel threads finished
	for (std::thread& t : threads) {
		t.join();
	}
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::cout << elapsed_seconds.count() << "s rendering\n";

	ppm_.save(filename_);
}

void Renderer::thread_function(Scene const& scene, float img_plane_dist, glm::mat4 const& trans_mat) {
	float aa_unit = 1.0f / aa_steps_;
	//continuously picks pixels to render
	while (true) {
		unsigned current_pixel = pixel_index_++;
		unsigned x = current_pixel % width_;
		unsigned y = current_pixel / width_;

		if (current_pixel >= width_ * height_) {
			return;
		}
		Color traced_color{};

		for (int aax = 0; aax < aa_steps_; ++aax) {
			for (int aay = 0; aay < aa_steps_; ++aay) {
				glm::vec3 pixel_pos = glm::vec3{
						x + aax * aa_unit - (width_ * 0.5f),
						y + aay * aa_unit - (height_ * 0.5f),
						-img_plane_dist};

				glm::vec4 trans_ray_dir = trans_mat * glm::vec4{ glm::normalize(pixel_pos), 0 };
				Ray ray{ glm::vec3{trans_mat[3]}, glm::vec3{trans_ray_dir} };
				traced_color += trace(ray, scene);
			}
		}
		traced_color *= 1.0f / (aa_steps_ * aa_steps_);
		Pixel pixel{ x, y };
		pixel.color = tone_map_color(traced_color);
		write(pixel);
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

Color Renderer::trace(Ray const& ray, Scene const& scene, unsigned ray_bounces) const {
	HitPoint closest_hit = scene.root->intersect(ray);
	return closest_hit.does_intersect ? shade(closest_hit, scene, ray_bounces) : Color {};
}

Color Renderer::shade(HitPoint const& hit_point, Scene const& scene, unsigned ray_bounces) const {
	auto material = hit_point.hit_material;
	Color shaded_color = phong_color(hit_point, scene) * material->opacity;

	if (ray_bounces >= max_ray_bounces_) {
		return shaded_color;
	}
	if (material->glossy > 0 && material->opacity < 1) {
		float reflectance = schlick_reflection_ratio(hit_point.ray_direction, hit_point.surface_normal, material->ior);
		shaded_color *= reflectance * material->opacity;
		shaded_color += reflection(hit_point, scene, ray_bounces) * reflectance;
		shaded_color += refraction(hit_point, scene, ray_bounces) * (1 - reflectance) * (1 - material->opacity);
	} else if (material->glossy > 0) {
		float reflectance = schlick_reflection_ratio(hit_point.ray_direction, hit_point.surface_normal, material->ior);
		reflectance = material->glossy + (1 - material->glossy) * reflectance;
		shaded_color *= 1 - reflectance;
		shaded_color += reflection(hit_point, scene, ray_bounces) * reflectance;
	} else if (material->opacity < 1) {
		shaded_color *= material->opacity;
		shaded_color += refraction(hit_point, scene, ray_bounces) * (1 - material->opacity);
	}
	return shaded_color;
}

Color Renderer::phong_color(HitPoint const& hit_point, Scene const& scene) const {
	auto material = hit_point.hit_material;
	//adds ambient light
	Color phong_color = scene.ambient.intensity * material->ka;

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
		phong_color += light.intensity * material->kd * cos_view_angle;

		//adds specular light
		if (material->m != 0) {
			phong_color += specular_color(hit_point.ray_direction, light_dir, normal, light.intensity, material);
		}
	}
	return phong_color;
}

HitPoint Renderer::find_light_block(Ray const& light_ray, float range, Scene const& scene) const {
	HitPoint hit = scene.root->intersect(light_ray);

	if (hit.does_intersect && hit.distance <= range) {
		return hit;
	}
	return HitPoint {};
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

Color Renderer::reflection(HitPoint const& hit_point, Scene const& scene, unsigned ray_bounces) const {
	glm::vec3 ray_dir = hit_point.ray_direction;
	glm::vec3 normal = hit_point.surface_normal;

	float cos_incoming = -glm::dot(normal, ray_dir);
	glm::vec3 new_dir = ray_dir + (normal * cos_incoming * 2.0f);
	return trace({hit_point.position, new_dir}, scene, ray_bounces + 1) * hit_point.hit_material->ks;
}

Color Renderer::refraction(HitPoint const& hit_point, Scene const& scene, unsigned ray_bounces) const {
	glm::vec3 ray_dir = hit_point.ray_direction;
	glm::vec3 normal = hit_point.surface_normal;
	float eta = 1 / hit_point.hit_material->ior;
	float cos_incoming = -glm::dot(normal, ray_dir);
	//inverts negative incoming angle and normal vector if surface is hit from behind
	if (cos_incoming < 0) {
		eta = 1 / eta;
		cos_incoming *= -1;
		normal *= -1;
	}
	float cos_outgoing_squared = 1 - eta * eta * (1 - cos_incoming * cos_incoming);

	//returns total reflection if critical angle is reached
	if (cos_outgoing_squared < 0) {
		return reflection(hit_point, scene, ray_bounces);
	}else {
		//glm::vec3 new_dir = glm::refract(ray_dir, normal, eta);
		glm::vec3 new_dir = ray_dir * eta + normal * (eta * cos_incoming - sqrtf(cos_outgoing_squared));
		Ray new_ray {hit_point.position - normal * (2 * EPSILON), new_dir};
		return trace(new_ray, scene, ray_bounces + 1) * hit_point.hit_material->kd;
	}
}

//https://en.wikipedia.org/wiki/Fresnel_equations
//https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel
float Renderer::fresnel_reflection_ratio(glm::vec3 const& ray_dir, glm::vec3 const& normal, float const& ior) const {
	float cos_in = glm::dot(normal, ray_dir);
	float eta_in = 1;
	float eta_out = ior;

	if (cos_in > 0) {
		std::swap(eta_in, eta_out);
	}
	float sin_out = eta_in / eta_out * sqrtf(1 - cos_in * cos_in);
	//total internal reflection
	if (sin_out >= 1) {
		return 1;
	} else {
		float cos_out = sqrtf(std::max(0.0f, 1 - sin_out * sin_out));
		cos_in = fabsf(cos_in);
		float reflectiveness_s_light = ((eta_out * cos_in) - (eta_in * cos_out)) / ((eta_out * cos_in) + (eta_in * cos_out));
		float reflectiveness_p_light = ((eta_in * cos_in) - (eta_out * cos_out)) / ((eta_in * cos_in) + (eta_out * cos_out));
		return (reflectiveness_s_light * reflectiveness_s_light + reflectiveness_p_light * reflectiveness_p_light) / 2;
	}
}

//https://blog.demofox.org/2017/01/09/raytracing-reflection-refraction-fresnel-total-internal-reflection-and-beers-law/
//https://en.wikipedia.org/wiki/Schlick%27s_approximation
float Renderer::schlick_reflection_ratio(glm::vec3 const& ray_dir, glm::vec3 const& normal, float const& ior) const {
	float n1 = 1;
	float n2 = ior;
	float cos_incoming = -glm::dot(normal, ray_dir);

	if (cos_incoming < 0) {
		std::swap(n1, n2);
	}
	if (n1 > n2) {
		float eta = n1 / n2;
		float sin_outgoing_squared = eta * eta * (1 - cos_incoming * cos_incoming);

		if (sin_outgoing_squared >= 1) {
			return 1;
		}
		cos_incoming = sqrtf(1 - sin_outgoing_squared);
	}

	float min_reflectance = (1 - ior) / (1 + ior);
	min_reflectance *= min_reflectance;

	float factor = 1 - cos_incoming;
	float ratio = min_reflectance + (1 - min_reflectance) * factor * factor * factor * factor * factor;
	return ratio;
}

Color Renderer::normal_color(HitPoint const& hit_point) const {
	return Color {
			(hit_point.surface_normal.x + 1) / 2,
			(hit_point.surface_normal.y + 1) / 2,
			(hit_point.surface_normal.z + 1) / 2
	};
}

Color Renderer::tone_map_color(Color color) const {
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





