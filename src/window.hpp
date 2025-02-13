#pragma once 

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

private:
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	void initWindow();

	int width;
	int height;
	bool framebufferResized = false;

	GLFWwindow* window;
};
