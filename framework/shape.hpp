#ifndef RAYTRACER_SHAPE_HPP
#define RAYTRACER_SHAPE_HPP

#define GLM_FORCE_RADIANS

#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "color.hpp"
#include "hitPoint.hpp"
#include "ray.hpp"
#include "material.hpp"
#include "printVec3.hpp"

class Shape {

public:
	Shape(std::string const& name, std::shared_ptr<Material> material);

	virtual std::string get_name() const;
	virtual float area() const = 0;
	virtual float volume() const = 0;
	virtual glm::vec3 min(glm::mat4 const& transform = glm::mat4()) const = 0;
	virtual glm::vec3 max(glm::mat4 const& transform = glm::mat4()) const = 0;
	virtual HitPoint intersect(Ray const& ray) const = 0;

	virtual void transform(glm::mat4 const& transformation);
	virtual void scale(float sx, float sy, float sz);
	void rotate(float yaw, float pitch, float roll);
	virtual void translate(float x, float y, float z);
	virtual std::ostream& print (std::ostream& os) const;

protected:
	std::string name_;
	std::shared_ptr<Material> material_;
	glm::mat4 world_transform_;
	glm::mat4 world_transform_inv_;
};

std::ostream& operator<<(std::ostream& os, Shape const& s);
glm::vec3 transform_vec(glm::vec3 const& vec, glm::mat4 const& transformation, bool is_location = true);
Ray transform_ray(Ray const& ray, glm::mat4 const& transformation);

#endif
