#include "shape.hpp"

Shape::Shape(std::string const& name, std::shared_ptr<Material> material) :
		name_{name},
		material_{material},
		world_transform_(glm::mat4(1.0f)),
		world_transform_inv_(glm::inverse(world_transform_)) {}

std::string Shape::get_name() const {
	return name_;
}

void Shape::transform(glm::mat4 const& transformation) {
	world_transform_ = transformation;
}

void Shape::scale(float sx, float sy, float sz) {
//	world_transform_[0][0] *= factor;
//	world_transform_[1][1] *= factor;
//	world_transform_[2][2] *= factor;
	world_transform_ = glm::scale(world_transform_, glm::vec3{sx, sy, sz});
	world_transform_inv_ = glm::inverse(world_transform_);
}

void Shape::rotate(float yaw, float pitch, float roll) {
	world_transform_ *= glm::eulerAngleYXZ(yaw, pitch, roll);
	world_transform_inv_ = glm::inverse(world_transform_);
}

void Shape::translate(float x, float y, float z) {
//	world_transform_[0][3] += x;
//	world_transform_[1][3] += y;
//	world_transform_[2][3] += z;
	world_transform_ = glm::translate(world_transform_, glm::vec3{x, y, z});
	world_transform_inv_ = glm::inverse(world_transform_);
}

std::ostream& Shape::print(std::ostream &os) const {
	return os << "=== " << name_ << " ===" << "\ncolor:"<< material_;
}

std::ostream& operator<<(std::ostream &os, Shape const& s) {
	return s.print(os);
}

glm::vec3 transform_vec(glm::vec3 const& vec, glm::mat4 const& transformation, bool is_location) {
	return glm::vec3{transformation * glm::vec4{vec, is_location}};
}

Ray transform_ray(Ray const& ray, glm::mat4 const& transformation) {
	return Ray{transform_vec(ray.origin, transformation), transform_vec(ray.direction, transformation, false)};
}
