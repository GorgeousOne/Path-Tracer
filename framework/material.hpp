
#ifndef RAYTRACER_MATERIAL_HPP
#define RAYTRACER_MATERIAL_HPP
#include <string>
#include <glm/glm.hpp>
#include "printVec3.hpp"
#include "color.hpp"

//Aufgabe 6.4
struct Material {
	std::string name = "default";
	//ambient color reflection coefficient
	Color ka {0.2, 0.2, 0.2};
	//diffuse color reflection coefficient
	Color kd {0.5, 0.5, 0.5};
	//specular reflection coefficient
	Color ks {};
	//specular reflection exponent
	float m = 0;

	float glossy = 0;

	float opacity = 1;

	//index of refraction
	float ior = 1;
};

std::ostream& operator<<(std::ostream& os, Material const& mat);

#endif
