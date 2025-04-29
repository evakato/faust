#pragma once 

#include <vector>
#include <string>

#include "model.hpp"

namespace faust {
	void loadFromFile(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Face>& faces);
	std::vector<char> readFile(const std::string& filename);
}

