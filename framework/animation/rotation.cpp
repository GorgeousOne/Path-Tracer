#include "rotation.hpp"

Rotation::Rotation(
		std::string const& shape,
		float t0,
		float dt,
		std::function<float(float)> const& transition,
		float angle0,
		float angle1,
		glm::vec3 const& axis) : Animation(shape, t0, dt, transition), angle0_{angle0}, rotation_{angle1 - angle0}, axis_{axis} {}

std::ostream &Rotation::print(std::ostream &os, float t) const {
	Animation::print(os, t);
	float percentage = transition_(t - t0_);
	os << angle0_ + percentage * rotation_ << " " << axis_.x << " " << axis_.y << " "<< axis_.z << std::endl;
	return os;
}