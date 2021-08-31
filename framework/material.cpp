#include "material.hpp"

std::ostream& operator<<(std::ostream &os, Material const& mat) {
	return os
	<< "(name:" << mat.name
	<< "\nkd" << mat.kd
	<< "\nks" << mat.ks << ")";
}
