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

	KeyPress detectKeypress();

private:
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	void initWindow();

	int width;
	int height;
	bool framebufferResized = false;

	GLFWwindow* window;
};
