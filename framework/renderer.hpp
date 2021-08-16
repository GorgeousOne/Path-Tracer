// -----------------------------------------------------------------------------
// Copyright  : (C) 2014-2017 Andreas-C. Bernstein
// License    : MIT (see the file LICENSE)
// Maintainer : Andreas-C. Bernstein <andreas.bernstein@uni-weimar.de>
// Stability  : experimental
//
// Renderer
// -----------------------------------------------------------------------------

#ifndef BUW_RENDERER_HPP
#define BUW_RENDERER_HPP

#include <string>
#include <glm/glm.hpp>
#include <atomic>
#include <random>
#include "color.hpp"
#include "pixel.hpp"
#include "ppmwriter.hpp"
#include "scene.hpp"

class Renderer {
public:
	template<typename T>
	using vector2d = std::vector<std::vector<T>>;

	Renderer(unsigned w, unsigned h, std::string const& file, unsigned pixel_samples, unsigned aa_samples, unsigned ray_bounces);

	void render(Scene const &scene);
	void thread_function(Scene const &scene, glm::mat4 const &cam_mat, float img_plane_dist);

	template<typename T>
	std::vector<T> concat_buffer(vector2d<T> const& buffer2d) const {
		std::vector<T> buffer1d;
		buffer1d.reserve(buffer2d.size() * buffer2d[0].size());

		for (int i = 0; i < width_; ++i) {
			buffer1d.insert(buffer1d.end(), buffer2d[i].begin(), buffer2d[i].end());
		}
		return buffer1d;
	}

	std::vector<Color> color_buffer() {
		std::vector<Color> buffer1d;
		unsigned total_samples = aa_samples_ * aa_samples_;
		buffer1d.reserve(width_ * height_ / total_samples);

		for (unsigned y = 0; y < width_; y += aa_samples_) {
			for (unsigned x = 0; x < height_; x += aa_samples_) {
				Color color_sum{};

				for (unsigned aay = 0; aay < aa_samples_; ++aay) {
					for (unsigned aax = 0; aax < aa_samples_; ++aax) {
						color_sum += color_buffer_[y + aay][x + aax];
					}
				}
				color_sum *= 1.0f / total_samples;
				buffer1d.emplace_back(color_sum);

				Pixel p = Pixel(x / aa_samples_, y / aa_samples_);
				p.color = color_sum;
				ppm_.write(p);
			}
		}
		return buffer1d;
	}

	std::vector<Color> normal_buffer() const {
		std::vector<glm::vec3> buffer1d = concat_buffer(normal_buffer_);
		std::vector<Color> color_buffer;
		color_buffer.reserve(buffer1d.size());

		for (glm::vec3& n : buffer1d) {
			color_buffer.emplace_back(Color::to_color((n + glm::vec3{1, 1, 1}) * 0.5f));
		}
		return color_buffer;
	}

	std::vector<Color> distance_buffer() const {
		std::vector<float> buffer1d = concat_buffer(distance_buffer_);
		std::vector<Color> color_buffer;
		color_buffer.reserve(buffer1d.size());

		for (float const& distance : buffer1d) {
			color_buffer.push_back(Color::gray_color(distance));
		}
		return color_buffer;
	}

private:
	unsigned width_;
	unsigned height_;
	unsigned pixel_samples_;
	unsigned aa_samples_;
	unsigned ray_bounces_;
	std::string filename_;
	PpmWriter ppm_;

	std::atomic_uint pixel_index_;
	std::atomic<float> progress;
	std::minstd_rand gen_;
	std::uniform_real_distribution<float> dist_;

	vector2d<Color> color_buffer_;
	vector2d<glm::vec3> normal_buffer_;
	vector2d<float> distance_buffer_;
	vector2d<std::shared_ptr<Material>> material_buffer_;

	Color primary_trace(unsigned x, unsigned y, Ray const& ray, Scene const& scene);
	Color trace(Ray const& ray, Scene const& scene, unsigned ray_bounces);
	HitPoint get_closest_hit(Ray const& ray, Scene const& scene) const;
	Color bounce_color(HitPoint const &hit_point, Scene const &scene, unsigned samples, unsigned ray_bounces);

	void denoise();

	Color tone_map_color(Color color) const;
};

#endif // #ifndef BUW_RENDERER_HPP
