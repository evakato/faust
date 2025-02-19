#include "math.hpp"

struct Light {
	glm::vec4 color{ 1.f };
};

struct PointLight : public Light {
	glm::vec4 position{ 2.0f, 1.0f, 0.0f, 1.0f };
	PointLight() {
		color = glm::vec4{ 0.f, 1.f, 0.f, 1.f };
	}
};

struct AmbientLight : public Light {};