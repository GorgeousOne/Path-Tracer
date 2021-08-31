
#ifndef RAYTRACER_MATERIAL_HPP
#define RAYTRACER_MATERIAL_HPP
#include <string>
#include <glm/glm.hpp>
#include "printVec3.hpp"
#include "color.hpp"

//Aufgabe 6.4
struct Material {
	std::string name = "default";
	//diffuse color reflection coefficient
	Color kd = {0.5, 0.5, 0.5};
	//specular reflection coefficient
	Color ks = {};

	float glossy = 0;

	float opacity = 1;

	float emittance = 0;

	float ior = 1;

	Color emit_color = kd * emittance;
};

std::ostream& operator<<(std::ostream& os, Material const& mat);

#endif
