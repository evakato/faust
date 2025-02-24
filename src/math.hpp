#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

constexpr float cameraTranslatePos{ 0.001f };
constexpr float cameraTranslateNeg{ -0.001f };
constexpr float cameraPan{ 0.1f };
