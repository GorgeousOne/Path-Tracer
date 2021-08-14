#include "kernel3.hpp"


glm::mat3 gaussian() {
	return (1.0f/16) * glm::mat3 {
			1, 2, 1,
			2, 4, 2,
			1, 2, 1
	};
}

void apply(glm::mat3 const& kernel, pixel_buffer const& src, pixel_buffer &dest) {
	for (int x = 0; x < src.size(); ++x) {
		for (int y = 0; y < src[0].size(); ++y) {
			apply(kernel, x, y, src, dest);
		}
	}
}

void apply(glm::mat3 const& kernel, int x, int y, pixel_buffer const& src, pixel_buffer &dest) {
	glm::vec3 result{};
	float determinant = 0;

	for (int dx = -1; dx <= 1; ++dx) {
		for (int dy = -1; dy <= 1; ++dy) {
			int rel_x = x + dx;
			int rel_y = y + dy;

			if (rel_x >= 0 && rel_x < src.size() &&
			    rel_y >= 0 && rel_y < src[0].size()) {
				float factor = kernel[dx + 1][dy + 1];
				result += factor * src[rel_x][rel_y];
				determinant += factor;
			}
		}
	}
	dest[x][y] = result * (1.0f / determinant);
}

void adjust(glm::mat3 &kernel, int x, int y, pixel_buffer const& image, std::function<float(glm::vec3, glm::vec3)> const& lambda) {
	glm::vec3 center = image[x][y];

	for (int dx = -1; dx <= 1; ++dx) {
		for (int dy = -1; dy <= 1; ++dy) {
			int rel_x = x + dx;
			int rel_y = y + dy;
			float factor = 0;

			if (rel_x >= 0 && rel_x < image.size() &&
			    rel_y >= 0 && rel_y < image[0].size()) {
				factor = lambda(center, image[x][y]);
			}
			kernel[dx + 1][dy + 1] *= factor;
		}
	}
}

