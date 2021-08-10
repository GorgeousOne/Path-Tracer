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
	Renderer(unsigned w, unsigned h, std::string const& file);

//	void render(Scene const& scene, Camera const& cam);
	void render_threaded(Scene const &scene, Camera const &cam);
//	void thread_function(Scene const &scene, glm::mat4 const& c, float min_x, float min_y, float img_dist);
	void thread_function(Scene const &scene, glm::mat4 const &c, float img_dist);

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

//	std::vector<glm::vec3> normal_sphere_;
	std::atomic_uint pixel_index_;
	std::atomic<float> progress;
	std::minstd_rand gen_;
	std::uniform_real_distribution<float> dist_;

	Color trace(Ray const& ray, Scene const& scene, unsigned ray_bounces);
	HitPoint get_closest_hit(Ray const& ray, Scene const& scene) const;
//	HitPoint find_light_block(Ray const& light_ray, float range, Scene const& scene) const;

//	Color shade(HitPoint const& hit_point, Scene const& scene, unsigned bounces, unsigned bounce_depth) const;
//	Color phong_color(HitPoint const& hitPoint, Scene const& scene) const;
//	Color specular_color(glm::vec3 const& viewer_dir, glm::vec3 const& light_dir, glm::vec3 const& normal,
//	                     Color const& light_intensity, std::shared_ptr<Material> material) const;

	Color normal_color(HitPoint const& hitPoint) const;
	Color& tone_map_color(Color &color) const;

	Color reflection(HitPoint const& hitPoint, Scene const& scene, unsigned bounces) const;

	Color bounce_color(HitPoint const &hit_point, Scene const &scene, unsigned ray_bounces);
//	std::vector<Ray> ray_hemisphere(glm::vec3 const &origin, glm::vec3 const &normal) const;
};

#endif // #ifndef BUW_RENDERER_HPP
