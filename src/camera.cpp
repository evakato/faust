#include "camera.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
	: position(position), up(glm::normalize(up)), front(glm::normalize(direction)) {
	// Calculate initial yaw and pitch from direction vector
	yaw = glm::degrees(atan2(front.z, front.x));
	pitch = glm::degrees(asin(front.y));

	updateVectors();
}

void Camera::setOrthographicProjection(
	float left, float right, float top, float bottom, float near, float far) {
	projectionMatrix = glm::mat4{ 1.0f };
	projectionMatrix[0][0] = 2.f / (right - left);
	projectionMatrix[1][1] = 2.f / (bottom - top);
	projectionMatrix[2][2] = 1.f / (far - near);
	projectionMatrix[3][0] = -(right + left) / (right - left);
	projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
	projectionMatrix[3][2] = -near / (far - near);
}

void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
	assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
	projectionMatrix = glm::perspective(fovy, aspect, near, far);
	projectionMatrix[1][1] *= -1;
}

void Camera::translate(glm::vec3 offset) {
	position += right * offset.x;
	position += up * offset.y;
	position += front * offset.z;
}

void Camera::rotate(float yawOffset, float pitchOffset) {
	yaw += yawOffset;
	pitch += pitchOffset;

	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	updateVectors();
}

void Camera::updateVectors() {
	glm::vec3 newFront;
	newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newFront.y = sin(glm::radians(pitch));
	newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(newFront);

	right = glm::normalize(glm::cross(front, up)); // Recalculate right vector
}