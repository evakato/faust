#pragma once 

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

static const char* shadingOptions[] = { "Normals", "Textured", "Diffuse" };
enum class ShadingSettings {
	Normals,
	Textured,
	Diffuse,
};

struct FaustState {
	static FaustState& getInstance() {
		static FaustState instance;
		return instance;
	}

	int numTris = 0;

	KeyPress currentKeyPress = KeyPress::None;

	ShadingSettings shadingSetting = ShadingSettings::Diffuse;

	std::string modelPath = "assets/models/vase.obj";
	std::string texturePath = "assets/textures/viking_room.png";
	bool modelChanged = false;

	// camera
	glm::vec3 cameraPos;
	glm::vec3 cameraViewDir;
	glm::vec3 cameraUpDir;
	// light
	glm::vec3 pointLightPos;
	glm::vec3 pointLightCol;

private:
	FaustState() {}  // Private constructor to enforce singleton pattern
};
