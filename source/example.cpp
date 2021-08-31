
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
	std::cout << "done\n";
//	Renderer renderer{img_width, img_height, "../../sdf/cornell-box.ppm", 100, 3, 5};
//
//	try {
//		auto start = std::chrono::steady_clock::now();
//		renderer.render(scene);
//		auto end = std::chrono::steady_clock::now();
//		std::chrono::duration<double> elapsed_seconds = end-start;
//		std::cout << elapsed_seconds.count() << "s\n";
//	} catch (const char *str) {
//		std::cout << "Exception: " << str << std::endl;
//	}
//
//	Window window{{img_width, img_height}};
//	auto buffer = renderer.color_buffer();
////	auto buffer = renderer.normal_buffer();
////	auto buffer = renderer.distance_buffer();
//
//	while (!window.should_close()) {
//		if (window.get_key(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
//			window.close();
//		}
//		window.show(buffer);
//	}
	return 0;
}
