#ifndef RAYTRACER_COMPOSITE_H
#define RAYTRACER_COMPOSITE_H

#include <vector>
#include <map>
#include "shape.hpp"
#include "box.hpp"

class Composite : public Shape {
public:
	Composite(std::string const& name, std::shared_ptr<Material> material = nullptr);
	Composite(std::shared_ptr<Box> bounds, std::string const& name, std::shared_ptr<Material> material = nullptr);

	float area() const override;
	float volume() const override;
	glm::vec3 min(glm::mat4 const& transform = glm::mat4()) const override;
	glm::vec3 max(glm::mat4 const& transform = glm::mat4()) const override;

	virtual void transform(glm::mat4 const& transformation) override;
	virtual void scale(float sx, float sy, float sz) override;
	virtual void rotate(float yaw, float pitch, float roll) override;
	virtual void translate(float x, float y, float z) override;

	std::ostream& print(std::ostream &os) const override;
	HitPoint intersect(Ray const& ray) const override;

	void add_child(std::shared_ptr<Shape> shape);
	unsigned int child_count();
	std::shared_ptr<Shape> find_child(std::string const& name) const;

	void build_octree();

private:
	std::shared_ptr<Box> bounds_;
	std::map<std::string, std::shared_ptr<Shape>> children_;
};

#endif //RAYTRACER_COMPOSITE_H
