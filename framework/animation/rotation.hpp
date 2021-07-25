#ifndef RAYTRACER_ROTATION_HPP
#define RAYTRACER_ROTATION_HPP

#include <glm/glm.hpp>
#include "animation/animation.hpp"

class Rotation : public Animation {
public:
	Rotation(std::string const& shape, float t0, float dt, std::function<float(float)> const& transition, float angle0, float angle1, glm::vec3 const& axis);
	std::ostream& print (std::ostream& os, float t) const override;
private:
	float angle0_;
	float rotation_;
	glm::vec3 axis_;
};

#endif //RAYTRACER_ROTATION_HPP
