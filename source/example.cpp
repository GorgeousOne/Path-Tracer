
#include <GL/glew.h>
#include <GLFW//glfw3.h>
#include <iostream>
#include <chrono>
#include <window.hpp>

#include "scene.hpp"
#include "renderer.hpp"

int main(int argc, const char** argv) {
	unsigned img_width = 1000;
	unsigned img_height = 1000;

	Scene scene = load_scene("../../sdf/example.sdf");
//	scene.camera.position = glm::vec3 {-10, 5, 0};
//	scene.camera.direction = glm::vec3 {1, 0, 0};

//	scene.camera.position = glm::vec3 {0, 11, 13};
//	scene.camera.position = glm::vec3 {0, 60, 20};

//	scene.camera.position = glm::vec3 {0, 20, 0};
//	scene.camera.direction = glm::vec3 {0, -1, 0};
//	scene.camera.up = glm::vec3 {0, 0, 1};

	Renderer renderer{img_width, img_height, "../../sdf/img.ppm"};
	std::cout << "shapes " << scene.shapes.size() << "\n";
	std::cout << "lights " << scene.lights.size() << "\n";

	try {
		auto start = std::chrono::steady_clock::now();
		renderer.render(scene, scene.camera);
		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = end-start;
		std::cout << elapsed_seconds.count() << "s\n";
	} catch (const char *str) {
		std::cout << "Exception: " << str << std::endl;
	}

	Window window{{img_width, img_height}};

	while (!window.should_close()) {
		if (window.get_key(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			window.close();
		}
		window.show(renderer.color_buffer());
	}
	return 0;
}
