
#ifndef RAYTRACER_KERNEL3_HPP
#define RAYTRACER_KERNEL3_HPP

#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include "color.hpp"

using pixel_buffer = std::vector<std::vector<Color>>;

glm::mat3 gaussian();
glm::mat3 adjust(glm::mat3 const& kernel, int y, int x, pixel_buffer const& image, const std::function<float(glm::vec3, glm::vec3)>& lambda);
void apply_kernel(glm::mat3 const& kernel, pixel_buffer const& src, pixel_buffer &dest);
void apply_kernel(glm::mat3 const& kernel, int y, int x, pixel_buffer const& src, pixel_buffer &dest);

#endif //RAYTRACER_KERNEL3_HPP
