#include "sphere.hpp"
#include <cmath>   //pow

#include <glm/glm.hpp>

#define PI 3.14159265f
#define EPSILON 0.001f

Sphere::Sphere(float radius, glm::vec3 const& center, std::string const& name, std::shared_ptr<Material> material) :
		Shape(name, material),
		radius_(std::abs(radius)),
		center_(center) {}

float Sphere::area() const {
	return 4.0f * PI * pow(radius_, 2);
}

float Sphere::volume() const {
	return 4.0f / 3.0f * PI * std::abs(pow(radius_, 3));
}

glm::vec3 Sphere::min(glm::mat4 const& transform) const {
	return center_ - glm::vec3 {radius_, radius_, radius_};
}

glm::vec3 Sphere::max(glm::mat4 const& transform) const {
	return center_ + glm::vec3 {radius_, radius_, radius_};
}

std::ostream& Sphere::print(std::ostream &os) const {
	Shape::print(os);
	return os << "\nradius: " << radius_ << "\ncenter: " << transform_vec(center_, world_transform_) << std::endl;
}

HitPoint Sphere::intersect(Ray const& ray) const {
	Ray ray_inv = transform_ray(ray, world_transform_inv_);
	float t;
	bool does_intersect = glm::intersectRaySphere(
			ray_inv.origin, glm::normalize(ray_inv.direction),
			center_,
			radius_ * radius_,
			t);

	if (does_intersect) {
		t -= EPSILON;
		glm::vec3 intersection = ray.point(t);
		return {does_intersect, t, name_, material_, intersection, ray.direction, surface_normal(intersection)};
	}else {
		return {};
	}
}

glm::vec3 Sphere::surface_normal(glm::vec3 const& intersection) const {
	return glm::normalize(intersection - transform_vec(center_, world_transform_));
}
