#include "window.hpp"

FaustWindow::FaustWindow(int w, int h) : width{ w }, height{ h } {
	initWindow();
}

FaustWindow::~FaustWindow() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

void FaustWindow::initWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void FaustWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto faustWindow = reinterpret_cast<FaustWindow*>(glfwGetWindowUserPointer(window));
	faustWindow->framebufferResized = true;
	faustWindow->width = width;
	faustWindow->height = height;
}
