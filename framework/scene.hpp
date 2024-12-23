#ifndef RAYTRACER_SCENE_HPP
#define RAYTRACER_SCENE_HPP

#include "shape.hpp"
#include "camera.hpp"
#include "pointLight.hpp"
#include "light.hpp"
#include "composite.hpp"
#include "triangle.hpp"
#include <vector>
#include <map>

struct Scene {
	std::shared_ptr<Composite> root = std::make_shared<Composite>("root");
	std::map<std::string, std::shared_ptr<Material>> materials{};
	std::vector<PointLight> lights{};
	Light ambient{};
	Camera camera{};

	std::shared_ptr<Material> find_mat(std::string const& name) const;
};


std::shared_ptr<Material> load_mat(std::istringstream& arg_stream);
void add_to_scene(std::istringstream& arg_stream, Scene& scene);
Scene load_scene(std::string const& file_path);

std::map<std::string, std::shared_ptr<Material>> load_obj_materials(std::string const& file_path);
std::shared_ptr<Triangle> load_obj_face(
		std::istringstream& arg_stream,
		std::vector<glm::vec3> const& vertices,
		std::vector<glm::vec3> const& normals,
		std::string const& name,
		std::shared_ptr<Material> mat);
std::shared_ptr<Composite> load_obj(std::string const& directory_path, std::string const& name);

#endif