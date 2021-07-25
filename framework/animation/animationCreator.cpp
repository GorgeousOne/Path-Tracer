
#include <string>
#include <fstream>
#include <memory>
#include <cmath>
#include <iomanip>
#include <iostream>
#include "animation.hpp"
#include "rotation.hpp"

#define PI 3.14159265f

template <typename T>
std::string left_pad_zeros(T const& input, unsigned zeros) {
	std::ostringstream stream;
	stream << std::setw(zeros) << std::setfill('0') << input;
	return stream.str();
}


template <typename T>
void create_sdf_files(std::string const& out_dir, T const& animations, float duration, unsigned fps) {
	unsigned frame_count = ceil(duration * fps);
	float frame_time = 1.0f / fps;

	for (int i = 0; i < frame_count; ++i) {
		float time = i * frame_time;
		std::string file_name = out_dir + "/scene" + left_pad_zeros(i+1, 4) + ".sdf";
		std::ofstream sdf_file (file_name);

		if (!sdf_file) {
			std::cerr << "Unable to open" + file_name << std::endl;
		}
		for (std::shared_ptr<Animation> const&  animation : animations) {
			if (animation->is_active(time)) {
				animation->print(std::cout, time);
				animation->print(sdf_file, time);
			}
		}
		sdf_file.close();
	}
}

int main(int argc, char** argv) {
	std::vector<std::shared_ptr<Animation>> movements;
	movements.push_back(std::make_shared<Rotation>("leg_left", 0, 1.5, [](float t) -> float {return -sin(4*t);}, -PI/16, PI/16, glm::vec3(1, 0, 0)));
	movements.push_back(std::make_shared<Rotation>("leg_right", 0, 1.5, [](float t) -> float {return sin(4*t);}, -PI/16, PI/16, glm::vec3(1, 0, 0)));
	create_sdf_files(".", movements, 3, 1);
	return 0;
}
