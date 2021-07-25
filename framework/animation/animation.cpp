#include "animation.hpp"

Animation::Animation(std::string const& shape, float t0, float dt, std::function<float(float)> const& transition) :
	shape_{shape}, t0_{t0}, dt_{dt}, transition_{transition} {}

bool Animation::is_active(float t) const {
	return t >= t0_ && t <= t0_ + dt_;
}

std::ostream &Animation::print(std::ostream &os, float time) const {
	os << "transform " << shape_ << " ";
	return os;
}
