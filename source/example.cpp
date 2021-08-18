
#include <GL/glew.h>
#include <iostream>
#include <chrono>
#include <window.hpp>

#include "scene.hpp"
#include "renderer.hpp"

int main(int argc, const char** argv) {
	unsigned img_width = 800;
	unsigned img_height = img_width;

	Scene scene = load_scene("../../sdf/example.sdf");
	Renderer renderer{img_width, img_height, "../../sdf/img.ppm", 5};
	std::cout << "shapes " << scene.root->child_count() << "\n";
	std::cout << "lights " << scene.lights.size() << "\n";

	try {
		auto start = std::chrono::steady_clock::now();
		renderer.render(scene);
		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = end-start;
		std::cout << elapsed_seconds.count() << "s\n";
	} catch (const char *str) {
		std::cout << "Exception: " << str << std::endl;
	}

	Window window{{img_width, img_height}};

	auto last_draw = std::chrono::steady_clock::now();
	double accumulator = 0;

	while (!window.should_close()) {
		if (window.get_key(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			window.close();
		}
		auto now = std::chrono::steady_clock::now();
		auto dTime = now - last_draw;
		accumulator += dTime.count();

		if (accumulator > 1) {
			window.show(renderer.color_buffer());
			--accumulator;
		}
	}
	return 0;
}
