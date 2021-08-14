
#ifndef RAYTRACER_KERNEL3_HPP
#define RAYTRACER_KERNEL3_HPP

#include <glm/glm.hpp>
#include <vector>
#include <functional>

using pixel_buffer = std::vector<std::vector<glm::vec3>>;

glm::mat3 gaussian();
void apply(glm::mat3 const& kernel, int x, int y, pixel_buffer src, pixel_buffer dest);
void adjust(glm::mat3 &kernel, int x, int y, std::vector<std::vector<glm::vec3>> const& image, const std::function<float(glm::vec3, glm::vec3)>& lambda);

#endif //RAYTRACER_KERNEL3_HPP
