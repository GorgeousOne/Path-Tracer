#ifndef RAYTRACER_POINTLIGHT_H
#define RAYTRACER_POINTLIGHT_H

#include "light.hpp"

struct PointLight {
	std::string name = "default";
	Color color {255, 255, 255};
	glm::vec3 position = {};
	float brightness = 1;
	Color intensity = color * brightness;
};
#endif
