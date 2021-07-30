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
#include "color.hpp"
#include "pixel.hpp"
#include "ppmwriter.hpp"
#include "scene.hpp"

class Renderer {
public:
	Renderer(unsigned w, unsigned h, std::string const& file);

	void render();
	void render(Scene const& scene, Camera const& cam);
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

	Color get_intersection_color(Ray const& ray, Scene const& scene) const;
	HitPoint get_closest_hit(Ray const& ray, Scene const& scene) const;
	HitPoint find_light_block(Ray const& light_ray, float range, Scene const& scene) const;

	Color intensity(Light const& light) const;
	Color shade(HitPoint const& hitPoint, Scene const& scene) const;
	Color phong_color(HitPoint const& hitPoint, Scene const& scene) const;
	Color specular_color(glm::vec3 const &viewer_dir, glm::vec3 const &light_dir, glm::vec3 const &normal,
	                     Color const &light_intensity, std::shared_ptr<Material> material) const;

	Color normal_color(HitPoint const& hitPoint) const;
	Color& tone_map_color(Color &color) const;
};

#endif // #ifndef BUW_RENDERER_HPP
