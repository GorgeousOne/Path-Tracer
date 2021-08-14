
#include <GL/glew.h>
#include <iostream>
#include <chrono>
#include <window.hpp>

#include "scene.hpp"
#include "renderer.hpp"

int main(int argc, const char** argv) {
	unsigned img_width = 400;
	unsigned img_height = img_width;

	Scene scene = load_scene("../../sdf/cornell-box.sdf");
	Renderer renderer{img_width, img_height, "../../sdf/cornell-box.ppm", 100, 5, 2};
	std::cout << "shapes " << scene.root->child_count() << "\n";
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
