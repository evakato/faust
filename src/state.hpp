#pragma once 

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

static const char* shadingOptions[] = { "Normals", "Textured", "Directional" };
enum class ShadingSettings {
	Normals,
	Textured,
	Directional,
};

struct FaustState {
	static FaustState& getInstance() {
		static FaustState instance;
		return instance;
	}

	int numTris = 0;

	KeyPress currentKeyPress = KeyPress::None;

	ShadingSettings shadingSetting = ShadingSettings::Textured;

private:
	FaustState() {}  // Private constructor to enforce singleton pattern
};
