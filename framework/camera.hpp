#ifndef RAYTRACER_CAMERA_H
#define RAYTRACER_CAMERA_H

#include <glm/vec3.hpp>
#include "color.hpp

struct Camera {
	glm::vec3 position = {};
	glm::vec3 direction = {};
	float fov_x = 60.0f;
};
#endif