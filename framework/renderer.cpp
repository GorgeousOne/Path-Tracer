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

Renderer::Renderer(unsigned w, unsigned h, std::string const& file, unsigned pixel_samples, unsigned ray_bounces, unsigned aa_samples) :
		width_(w),
		height_(h),
		pixel_samples_(pixel_samples),
		ray_bounces_(ray_bounces),
		aa_samples_(aa_samples),
		filename_(file),
		ppm_(width_, height_),
		color_buffer_(w * h, Color{}),
		normal_buffer_(w * aa_samples, std::vector<glm::vec3>(h * aa_samples, glm::vec3{})),
		distance_buffer_(w * aa_samples, std::vector<glm::vec3>(h * aa_samples, glm::vec3{})) {

	gen_ = std::minstd_rand(std::random_device{}());
	dist_ = std::uniform_real_distribution<float>(-1, 1);
}

void Renderer::render(Scene const& scene, Camera const& cam) {
	auto start = std::chrono::steady_clock::now();

	glm::vec4 u = glm::vec4(glm::cross(cam.direction, cam.up), 0);
	glm::vec4 v = glm::vec4(glm::cross({u.x, u.y, u.z}, cam.direction), 0);
	glm::mat4 c {u, v, glm::vec4{-cam.direction, 0}, glm::vec4{cam.position, 1}};

	float img_plane_dist = (width_ / 2.0f) / tan(cam.fov_x / 2);
	pixel_index_ = 0;

	std::cout << std::fixed;
	std::cout << std::setprecision(2);

	const size_t thread_count = std::thread::hardware_concurrency();
	std::cout << "using " << thread_count << " threads to render\n";

	std::vector<std::thread> threads;
	threads.resize(thread_count);

	for (std::thread& t : threads) {
		t = std::thread(&Renderer::thread_function, this, scene, c, img_plane_dist);
	}
	for (std::thread& t : threads) {
		t.join();
	}
	ppm_.save(filename_);
	std::cout << "save " << filename_ << "\n";
}

void Renderer::thread_function(Scene const& scene, glm::mat4 const& c,float img_plane_dist) {
	while(true) {
		unsigned current_index = pixel_index_++;

		if (current_index >= width_ * height_) {
			return;
		}
		unsigned x = current_index / width_;
		unsigned y = current_index % width_;
		float aa_step = 1.0f / aa_samples_;
		Color aa_color{};

		for (int aax = 0; aax < aa_samples_; ++aax) {
			for (int aay = 0; aay < aa_samples_; ++aay) {
				glm::vec3 pixel_pos = glm::vec3{
					width_ * -0.5f + x + aax * aa_step,
					height_ * -0.5f + y + aay * aa_step,
					-img_plane_dist};
				glm::vec4 trans_ray_dir = c * glm::vec4 {glm::normalize(pixel_pos), 0};
				Ray ray {glm::vec3{c[3]}, glm::vec3{trans_ray_dir}};
				aa_color += primary_trace(x, y, aax, aay, ray, scene);
			}
		}
		aa_color *= 1.0f / aa_samples_;
		Pixel pixel {x, y};
		pixel.color = tone_map_color(aa_color);
		write(pixel);

		if (y == 0 && x >= (progress + 0.01) * width_) {
			std::cout << static_cast<int>(100 * progress) << std::endl;
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
Color Renderer::primary_trace(unsigned x, unsigned y, unsigned aax, unsigned aay, Ray const& ray, Scene const& scene) {
	HitPoint closest_hit = get_closest_hit(ray, scene);

	if (closest_hit.does_intersect) {
		unsigned pos_x = x * aa_samples_ + aax;
		unsigned pos_y = y * aa_samples_ + aay;
		normal_buffer_[pos_x][pos_y] = closest_hit.surface_normal;
		distance_buffer_[pos_x][pos_y] = glm::vec3{closest_hit.distance};
	}
	return closest_hit.does_intersect ? bounce_color(closest_hit, scene, 0) : Color {};

}

Color Renderer::trace(Ray const& ray, Scene const& scene, unsigned ray_bounces) {
	HitPoint closest_hit = get_closest_hit(ray, scene);
	return closest_hit.does_intersect ? bounce_color(closest_hit, scene, ray_bounces) : Color {};
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

Color Renderer::bounce_color(HitPoint const& hit_point, Scene const& scene, unsigned ray_bounces) {

	if (ray_bounces >= ray_bounces_) {
		return {};
	}
	auto material = hit_point.hit_material;
	unsigned samples = ray_bounces == 0 ? pixel_samples_ / aa_samples_ : 1;
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

//float* Renderer::pixel_buffer() const {
//	auto* pixel_data = new float[width_ * height_ * 3];
//
//	for (int i = 0; i < color_buffer_.size(); ++i) {
//		Color color = color_buffer_[i];
//		pixel_data[i * 3] = color.r;
//		pixel_data[i * 3 + 1] = color.g;
//		pixel_data[i * 3 + 2] = color.b;
//	}
//	return pixel_data;
//}
