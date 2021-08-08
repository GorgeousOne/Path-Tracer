#include <numeric>
#include <iomanip>
#include "composite.hpp"

#define EPSILON 0.001f

Composite::Composite(std::string const& name, std::shared_ptr<Material> material) :
	Shape(name, material), bounds_{nullptr} {}

Composite::Composite(std::shared_ptr<Box> bounds, std::string const& name, std::shared_ptr<Material> material) :
	Shape(name, material), bounds_{bounds} {}

float Composite::area() const {
	float area_sum = 0;
	for (auto const& it : children_) {
		area_sum += it.second->area();
	}
	return area_sum;
}

float Composite::volume() const {
	float volume_sum = 0;

	for (auto const& it : children_) {
		volume_sum += it.second->volume();
	}
	return volume_sum;
}

glm::vec3 Composite::min(glm::mat4 const& transform) const {
	if (nullptr != bounds_) {
		return bounds_->min(transform);
	}
	glm::vec3 min{};
	bool is_first = true;
	
	for (auto const& it : children_) {
		if (is_first) {
			min = it.second->min(transform * world_transform_);
			is_first = false;
		}else {
			min = glm::min(min, it.second->min(transform * world_transform_));
		}
	}
	return min;
}

glm::vec3 Composite::max(glm::mat4 const& transform) const {
	if (nullptr != bounds_) {
		return bounds_->max(transform);
	}
	glm::vec3 max{};
	bool is_first = true;

	for (auto const& it : children_) {
		if (is_first) {
			max = it.second->max(transform * world_transform_);
			is_first = false;
		}else {
			max = glm::max(max, it.second->max(transform * world_transform_));
		}
	}
	return max;
}

std::ostream &Composite::print(std::ostream &os) const {
	Shape::print(os);

	for (auto const& it : children_) {
		os << it.second;
	}
	return os;
}

HitPoint Composite::intersect(Ray const& ray) const {
//	assert(nullptr != bounds_);
//	float t;
//	bool bounds_hit = bounds_->intersect(ray, t);
//
//	if (!bounds_hit) {
//		return HitPoint{};
//	}
	Ray ray_inv = transform_ray(ray, world_transform_inv_);
	float min_t = -1;
	HitPoint min_hit {};

	for (auto const& it : children_) {
		HitPoint hit = it.second->intersect(ray_inv);

		if (!hit.does_intersect) {
			continue;
		}
		if (!min_hit.does_intersect || hit.distance < min_t) {
			min_t = hit.distance;
			min_hit = hit;
		}
	}
//	if (min_hit.does_intersect) {
//		min_hit.position = transform_vec(min_hit.position, world_transform_);
//		min_hit.surface_normal = glm::normalize(transform_vec(min_hit.surface_normal, world_transform_, false));
//	}
	return min_hit;
}

void Composite::add_child(std::shared_ptr<Shape> shape) {
	if (children_.end() != children_.find(shape->get_name())) {
		std::cout << shape->get_name();
	}
	children_.emplace(shape->get_name(), shape);
}

std::shared_ptr<Shape> Composite::find_child(std::string const& name) const {
	return children_.find(name)->second;
}

unsigned Composite::child_count() {
	return children_.size();
}

void Composite::build_octree() {
	bounds_ = nullptr;
	bounds_ = std::make_shared<Box>(min(), max());
//	bounds_ = std::make_shared<Box>(min(), max(), "bound", std::make_shared<Material>());

	if (children_.size() <= 64) {
		return;
	}
	glm::vec3 oct_size = (max() - min()) * 0.5f;
	std::vector<std::shared_ptr<Composite>> subdivisions;

	for (int x = 0; x < 2; ++x) {
		for (int y = 0; y < 2; ++y) {
			for (int z = 0; z < 2; ++z) {
				glm::vec3 oct_min = min() + glm::vec3 {x * oct_size.x, y * oct_size.y, z * oct_size.z};
				glm::vec3 oct_max = min() + glm::vec3 {(x + 1) * oct_size.x, (y + 1) * oct_size.y, (z + 1) * oct_size.z};
				std::string index_str = std::to_string(z + 2*y + 4*x);
				subdivisions.push_back(std::make_shared<Composite>(Composite{std::make_shared<Box>(Box{oct_min, oct_max}), index_str}));
			}
		}
	}
	for (auto const& oct : subdivisions) {
		for (auto const& it : children_) {
			if (oct->bounds_->intersects_bounds(it.second)) {
				oct->add_child(it.second);
			}
		}
	}
	for (auto const& oct : subdivisions) {
		if (oct->children_.size() == children_.size()) {
			return;
		}
	}
	children_.clear();

	for (auto const& oct : subdivisions) {
		if (!oct->children_.empty()) {
			oct->build_octree();
			add_child(oct);
		}
	}
}

void Composite::transform(glm::mat4 const& transformation) {
	Shape::transform(transformation);
	build_octree();
}

void Composite::scale(float sx, float sy, float sz) {
	Shape::scale(sx, sy, sz);
	build_octree();
}

void Composite::rotate(float yaw, float pitch, float roll) {
	Shape::rotate(yaw, pitch, roll);
	build_octree();
}

void Composite::translate(float x, float y, float z) {
	Shape::translate(x, y, z);
	build_octree();
}
