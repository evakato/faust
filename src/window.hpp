#pragma once 

#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "state.hpp"

class FaustWindow {

public:
	FaustWindow(int w, int h);
	~FaustWindow();
	bool shouldClose() { return glfwWindowShouldClose(window); }
	VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
	bool wasWindowResized() const { return framebufferResized; }
	void resetWindowResizedFlag() { framebufferResized = false; }
	GLFWwindow* getGLFWwindow() const { return window; }
	bool isMinimized() const { return width == 0 || height == 0; }

	void pollEvents() {
		mouseDelta = glm::vec2{ 0.0, 0.0 };
		glfwPollEvents();
	}
	glm::vec2 detectMouseMovement() const {
		return 0.05f * mouseDelta;
	}

	KeyPress detectKeypress();

private:
	void initWindow();

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

	int width;
	int height;
	bool framebufferResized = false;
	GLFWwindow* window;

	bool isDragging = false;
	glm::vec2 mouseDelta{ 0.0 };
	glm::vec2 mousePos{ 0.0 };
};
