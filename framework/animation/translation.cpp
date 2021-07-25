#include "translation.hpp"

Translation::Translation(
		std::string const& shape,
		float t0,
		float dt,
		std::function<float(float)> const& transition,
		glm::vec3 const& pos0,
		glm::vec3 const& pos1) : Animation(shape, t0, dt, transition), pos0_{pos0}, translation_{pos1 - pos0} {}

std::ostream &Translation::print(std::ostream &os, float t) const {
	Animation::print(os, t);
	float percentage = transition_(t - t0_);
	os << pos0_.x + percentage * translation_.x << " "
		<< pos0_.y + percentage * translation_.y << " "
		<< pos0_.z + percentage * translation_.z << std::endl;
	return os;
}