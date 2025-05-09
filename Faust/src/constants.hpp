#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace faust {
	constexpr int WIDTH = 1024;
	constexpr int HEIGHT = 1024;
	constexpr int MAX_DESC_SETS = 8;
}
