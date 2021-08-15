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
#include "kernel3.hpp"

#define PI 3.14159265f

Renderer::Renderer(unsigned w, unsigned h, std::string const& file, unsigned pixel_samples, unsigned ray_bounces, unsigned aa_samples) :
		width_(w * aa_samples),
		height_(h * aa_samples),
		pixel_samples_(pixel_samples),
		ray_bounces_(ray_bounces),
		aa_samples_(aa_samples),
		filename_(file),
		ppm_(w, h),
		color_buffer_(height_, std::vector<Color>(width_, Color{})),
		normal_buffer_(height_, std::vector<glm::vec3>(width_, glm::vec3{0.5})),
		distance_buffer_(height_, std::vector<float>(width_, 1)),
		material_buffer_(height_, std::vector<std::shared_ptr<Material>>(width_, nullptr)) {

	gen_ = std::minstd_rand(std::random_device{}());
	dist_ = std::uniform_real_distribution<float>(-1, 1);
}

void Renderer::render(Scene const& scene, Camera const& cam) {
	auto start = std::chrono::steady_clock::now();

	glm::vec4 u = glm::vec4(glm::cross(cam.direction, cam.up), 0);
	glm::vec4 v = glm::vec4(glm::cross({u.x, u.y, u.z}, cam.direction), 0);
	glm::mat4 cam_mat {u, v, glm::vec4{-cam.direction, 0}, glm::vec4{cam.position, 1}};

	float img_plane_dist = (width_ / 2.0f) / tan(cam.fov_x / 2);
	pixel_index_ = 0;

	std::cout << std::fixed;
	std::cout << std::setprecision(2);

	const size_t thread_count = std::thread::hardware_concurrency();
	std::cout << "using " << thread_count << " threads to render\n";

	std::vector<std::thread> threads;
	threads.resize(thread_count);

	for (std::thread& t : threads) {
		t = std::thread(&Renderer::thread_function, this, scene, cam_mat, img_plane_dist);
	}
	for (std::thread& t : threads) {
		t.join();
	}
	denoise();
	denoise();
	denoise();
}

void Renderer::thread_function(Scene const& scene, glm::mat4 const& cam_mat, float img_plane_dist) {
	while(true) {
		unsigned current_index = pixel_index_++;
		unsigned x = current_index / width_;
		unsigned y = current_index % width_;

		if (current_index >= width_ * height_) {
			return;
		}
		glm::vec3 pixel_pos = glm::vec3{
			width_ * -0.5f + x,
			height_ * -0.5f + y,
			-img_plane_dist};

		glm::vec4 trans_ray_dir = cam_mat * glm::vec4 {glm::normalize(pixel_pos), 0};
		Ray ray {glm::vec3{cam_mat[3]}, glm::vec3{trans_ray_dir}};
		color_buffer_[y][x] = tone_map_color(primary_trace(y, x, ray, scene));

		if (y == 0 && x >= (progress + 0.01) * width_) {
			std::cout << static_cast<int>(100 * progress) << std::endl;
			progress = progress + 0.01f;
		}
	}
}

void Renderer::denoise() {
	vector2d<Color> result(height_, std::vector<Color>(width_));

	auto normal_adjustment = [](glm::vec3 const& center, glm::vec3 const& relative) {
		float dot = glm::dot(center, relative);
		return fmax(0, dot);
	};
	auto distance_adjustment = [](float center, float relative) {
		return 1 - abs(relative - center);
	};
	auto material_adjustment = [](std::shared_ptr<Material> center, std::shared_ptr<Material> relative) {
		return relative == center;
	};

	for (unsigned y = 0; y < color_buffer_.size(); ++y) {
		for (unsigned x = 0; x < color_buffer_.size(); ++x) {
			glm::mat3 gaussian = gaussian_blur();
			adjust_kernel<glm::vec3>(gaussian, y, x, normal_buffer_, normal_adjustment);
			adjust_kernel<float>(gaussian, y, x, distance_buffer_, distance_adjustment);
			adjust_kernel<std::shared_ptr<Material>>(gaussian, y, x, material_buffer_, material_adjustment);
			apply_kernel(gaussian, y, x, color_buffer_, result);
		}
	}
	color_buffer_ = result;
}

Color Renderer::primary_trace(unsigned y, unsigned x, Ray const& ray, Scene const& scene) {
	HitPoint closest_hit = get_closest_hit(ray, scene);

	if (!closest_hit.does_intersect) {
		return Color{};
	}
	normal_buffer_[y][x] = closest_hit.surface_normal;
	distance_buffer_[y][x] = closest_hit.distance;
	material_buffer_[y][x] = closest_hit.hit_material;
	return bounce_color(closest_hit, scene, pixel_samples_ / (aa_samples_ * aa_samples_), 0);
}

Color Renderer::trace(Ray const& ray, Scene const& scene, unsigned ray_bounces) {
	HitPoint closest_hit = get_closest_hit(ray, scene);
	return closest_hit.does_intersect ? bounce_color(closest_hit, scene, 1, ray_bounces) : Color {};
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

template <class T, class T2>
glm::vec3 uniform_normal(T& distribution, T2& generator) {
	float yaw = distribution(generator) * PI;
	float pitch = asin(distribution(generator));
	float cos_pitch = cos(pitch);

	return glm::vec3{
			cos_pitch * cos(yaw),
			sin(pitch),
			cos_pitch * sin(yaw)
	};
}

Color Renderer::bounce_color(HitPoint const& hit_point, Scene const& scene, unsigned samples, unsigned ray_bounces) {
	if (ray_bounces >= ray_bounces_) {
		return {};
	}
	auto material = hit_point.hit_material;
	Color bounced_light {};

	for (unsigned i = 0; i < samples; ++i) {
		glm::vec3 bounce_dir;

		if (material->glossiness != 0) {
			bounce_dir = glm::reflect(hit_point.ray_direction, hit_point.surface_normal);
		}else {
			bounce_dir = uniform_normal(dist_, gen_);
		}
		float cos_theta = glm::dot(hit_point.surface_normal, bounce_dir);

		if (cos_theta < 0) {
			bounce_dir *= -1;
			cos_theta *= -1;
		}
		bounced_light += material->emit_color + trace({hit_point.position, bounce_dir}, scene, ray_bounces + 1) * material->kd * 2 * cos_theta;
	}
	if (samples > 1) {
		bounced_light *= 1.0f / samples;
	}
	return bounced_light;
}

Color Renderer::tone_map_color(Color color) const {
	color.r /= color.r + 1;
	color.g /= color.g + 1;
	color.b /= color.b + 1;
	return color;
}