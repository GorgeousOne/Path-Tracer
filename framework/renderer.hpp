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
#include <thread>
#include "color.hpp"
#include "pixel.hpp"
#include "ppmwriter.hpp"
#include "scene.hpp"

class Renderer {
public:
	Renderer(unsigned w, unsigned h, std::string const& file, unsigned max_ray_bounces);

	void render(Scene const& scene);
	void write(Pixel const& p);

	inline std::vector<Color> const& color_buffer() const {
		return color_buffer_;
	}
	[[nodiscard]] float* pixel_buffer() const;

private:
	unsigned width_;
	unsigned height_;
	std::vector<Color> color_buffer_;
	std::string filename_;
	PpmWriter ppm_;
	unsigned max_ray_bounces_;

	std::atomic_uint pixel_index_;
	void thread_function(Scene const& scene, float img_plane_dist, glm::mat4 const& trans_mat);

	Color trace(Ray const& ray, Scene const& scene, unsigned ray_bounces = 0) const;
	HitPoint find_light_block(Ray const& light_ray, float range, Scene const& scene) const;

	Color shade(HitPoint const& hit_point, Scene const& scene, unsigned ray_bounces = 0) const;
	Color phong_color(HitPoint const& hitPoint, Scene const& scene) const;
	Color specular_color(glm::vec3 const& viewer_dir, glm::vec3 const& light_dir, glm::vec3 const& normal,
	                     Color const& light_intensity, std::shared_ptr<Material> material) const;

	Color normal_color(HitPoint const& hitPoint) const;
	Color tone_map_color(Color color) const;

	Color reflection(HitPoint const& hitPoint, Scene const& scene, unsigned bounces) const;

	Color refraction(const HitPoint &hit_point, const Scene &scene, unsigned int ray_bounces) const;
};

#endif // #ifndef BUW_RENDERER_HPP
