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

KeyPress FaustWindow::detectKeypress() {
	if (FaustState::getInstance().currentKeyPress != KeyPress::None)
		return KeyPress::None;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		return KeyPress::CameraLeft;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		return KeyPress::CameraRight;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		return KeyPress::CameraForward;
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		return KeyPress::CameraBackward;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		return KeyPress::CameraUp;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		return KeyPress::CameraDown;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		return KeyPress::CameraViewUp;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		return KeyPress::CameraViewDown;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		return KeyPress::CameraViewRight;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		return KeyPress::CameraViewLeft;
	}
	return KeyPress::None;
}
