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
	using pixel_buffer = std::vector<std::vector<Color>>;

	Renderer(unsigned w, unsigned h, std::string const& file, unsigned pixel_samples, unsigned ray_bounces, unsigned aa_samples);

	void render(Scene const &scene, Camera const &cam);
	void thread_function(Scene const &scene, glm::mat4 const &c, float img_plane_dist);

	void write(Pixel const& p);

	std::vector<Color> concat_buffer(pixel_buffer const& buffer2d) const {
		std::vector<Color> buffer1d;
		buffer1d.reserve(buffer2d.size() * buffer2d[0].size());

		for (int i = 0; i < width_; ++i) {
			buffer1d.insert(buffer1d.end(), buffer2d[i].begin(), buffer2d[i].end());
		}
		return buffer1d;
	}

	std::vector<Color> color_buffer() const {
		return concat_buffer(color_buffer_);
	}

	std::vector<Color> normal_buffer() const {
		std::vector<Color> buffer1d = concat_buffer(normal_buffer_);

		for (Color& c : buffer1d) {
			c += 1;
			c *= 0.5f;
		}
		return buffer1d;
	}

	std::vector<Color> distance_buffer() const {
		return concat_buffer(distance_buffer_);
	}

	//	[[nodiscard]] float* pixel_buffer() const;

private:
	unsigned width_;
	unsigned height_;
	unsigned pixel_samples_;
	unsigned ray_bounces_;
	unsigned aa_samples_;
	std::string filename_;
	PpmWriter ppm_;

	std::atomic_uint pixel_index_;
	std::atomic<float> progress;
	std::minstd_rand gen_;
	std::uniform_real_distribution<float> dist_;

	pixel_buffer color_buffer_;
	pixel_buffer normal_buffer_;
	pixel_buffer distance_buffer_;

	Color primary_trace(unsigned x, unsigned y, unsigned aax, unsigned aay, Ray const& ray, Scene const& scene);
	Color trace(Ray const& ray, Scene const& scene, unsigned ray_bounces);
	HitPoint get_closest_hit(Ray const& ray, Scene const& scene) const;
	Color bounce_color(HitPoint const &hit_point, Scene const &scene, unsigned samples, unsigned ray_bounces);

	void denoise();

	Color normal_color(HitPoint const& hitPoint) const;
	Color& tone_map_color(Color &color) const;

};

#endif // #ifndef BUW_RENDERER_HPP
