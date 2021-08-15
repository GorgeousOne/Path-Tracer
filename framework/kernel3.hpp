
#ifndef RAYTRACER_KERNEL3_HPP
#define RAYTRACER_KERNEL3_HPP

#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include "color.hpp"

template<typename T>
using vector2d = std::vector<std::vector<T>>;

glm::mat3 gaussian_blur() {
	return (1.0f/16) * glm::mat3 {
			1, 2, 1,
			2, 4, 2,
			1, 2, 1
	};
}
/**
 * Returns a copy of the kernel. Each element is multiplied with a factor calculated by the lambda.
 * The lambda is given the color of the center pixel and the color of the relative pixel.
 * @param kernel
 * @param y
 * @param x
 * @param image
 * @param lambda
 * @return
 */
template<class T>
void adjust_kernel(glm::mat3& kernel, int y, int x, vector2d<T> const& src, const std::function<float(T, T)>& lambda) {
	T center = src[y][x];

	for (int dy = -1; dy <= 1; ++dy) {
		for (int dx = -1; dx <= 1; ++dx) {

			if (dx == 0 && dy == 0) {
				continue;
			}
			int rel_y = y + dy;
			int rel_x = x + dx;
			float factor = 0;

			if (rel_y >= 0 && rel_y < src.size() &&
			    rel_x >= 0 && rel_x < src[0].size()) {
				factor = lambda(center, src[rel_y][rel_x]);
			}
			kernel[dy + 1][dx + 1] *= factor;
		}
	}
}

void apply_kernel(glm::mat3 const& kernel, int y, int x, vector2d<Color> const& src, vector2d<Color>& dest) {
	Color result{};
	float determinant = 0;

	for (int dy = -1; dy <= 1; ++dy) {
		for (int dx = -1; dx <= 1; ++dx) {
			int rel_y = y + dy;
			int rel_x = x + dx;

			if (rel_y >= 0 && rel_y < src.size() &&
			    rel_x >= 0 && rel_x < src[0].size()) {
				float factor = kernel[dy + 1][dx + 1];
				result += src[rel_y][rel_x] * factor;
				determinant += factor;
			}
		}
	}
	dest[y][x] = result * (1.0f / determinant);
}

void apply_kernel(glm::mat3 const& kernel, vector2d<Color> const& src, vector2d<Color>& dest) {
	for (int y = 0; y < src.size(); ++y) {
		for (int x = 0; x < src[0].size(); ++x) {
			apply_kernel(kernel, y, x, src, dest);
		}
	}
}

#endif //RAYTRACER_KERNEL3_HPP