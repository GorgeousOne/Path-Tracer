#ifndef RAYTRACER_SPHERE_HPP
#define RAYTRACER_SPHERE_HPP

#include "shape.hpp"

class Sphere : public Shape {

public:

	Sphere(float radius = 1.0f, glm::vec3 const& center = glm::vec3(0.0), std::string const& name = "sphere", std::shared_ptr<Material> material = nullptr);

	float area() const override;
	float volume() const override;
	glm::vec3 min(glm::mat4 const& transform = glm::mat4()) const override;
	glm::vec3 max(glm::mat4 const& transform = glm::mat4()) const override;

	std::ostream& print(std::ostream &os) const override;
	HitPoint intersect(Ray const& ray) const override;

private:
	float radius_;
	glm::vec3 center_;

	glm::vec3 surface_normal(glm::vec3 const& intersection) const;
};

#endif
