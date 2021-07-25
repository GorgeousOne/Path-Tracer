#ifndef RAYTRACER_ROTATION_HPP
#define RAYTRACER_ROTATION_HPP

#include <glm/glm.hpp>
#include "animation/animation.hpp"

class Translation : public Animation {
public:
	Translation(std::string const& shape, float t0, float dt, std::function<float(float)> const& transition, glm::vec3 const& pos0, glm::vec3 const& pos1);
	std::ostream& print (std::ostream& os, float t) const override;

private:
	glm::vec3 pos0_;
	glm::vec3 translation_;
};

#endif //RAYTRACER_ROTATION_HPP
