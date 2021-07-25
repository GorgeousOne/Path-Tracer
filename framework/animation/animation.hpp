#ifndef RAYTRACER_ANIMATION_HPP
#define RAYTRACER_ANIMATION_HPP

#include <ostream>
#include <functional>

class Animation {
public:
	/**
	 * Class for calculating transformations of objects for frames of an animation
	 * @param shape name of shape to animate
	 * @param t0 start time of animation
	 * @param dt duration of animation
	 * @param transition function that receives a delta time and returns values between 0 and 1 for transitioning between 2 states of transformation
	 */
	Animation(std::string const& shape, float t0, float dt, std::function<float(float)> const& transition);
	bool is_active(float t) const;
	virtual std::ostream& print (std::ostream& os, float time) const;

protected:
	std::string shape_;
	float t0_;
	float dt_;
	std::function<float(float)> transition_;
};

#endif //RAYTRACER_ANIMATION_HPP
