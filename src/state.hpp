#pragma once 

#include "imgui.h"

#include "math.hpp"

enum class KeyPress {
	None,
	CameraLeft,
	CameraRight,
	CameraUp,
	CameraDown,
	CameraForward,
	CameraBackward,
	CameraViewUp,
	CameraViewDown,
	CameraViewRight,
	CameraViewLeft,
};

static const char* shadingOptions[] = { "Normals", "Textured", "Diffuse", "Specular" };
enum class ShadingSettings {
	Normals,
	Textured,
	Diffuse,
	Specular,
};

struct FaustState {
	static FaustState& getInstance() {
		static FaustState instance;
		return instance;
	}

	int numTris = 0;

	KeyPress currentKeyPress = KeyPress::None;

	ShadingSettings shadingSetting = ShadingSettings::Textured;

	std::string modelPath = "assets/models/viking_room.obj";
	std::string texturePath = "assets/textures/viking_room.png";
	bool modelChanged = false;

	// camera
	glm::vec3 cameraPos;
	glm::vec3 cameraViewDir;
	glm::vec3 cameraUpDir;
	// light
	glm::vec3 pointLightPos;
	ImVec4 pointLightCol{ 1.0f, 1.0f, 1.0f, 1.0f };

	void setPointLightCol(glm::vec4 color) {
		pointLightCol = ImVec4(color.x, color.y, color.z, color.w);
	}
	glm::vec4 getPointLightCol() const {
		return glm::vec4{ pointLightCol.x, pointLightCol.y, pointLightCol.z, pointLightCol.w };
	}

	bool useMsaa = true;

private:
	FaustState() {}  // Private constructor to enforce singleton pattern
};
