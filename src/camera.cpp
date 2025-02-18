#include "camera.hpp"

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

void Camera::setCamera(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
	camPosition = position;
	viewDirection = direction;
	upDirection = up;
	setViewMatrix();
}

void Camera::translate(glm::vec3 distance) {
	camPosition += distance;
	setViewMatrix();
}

void Camera::pan(glm::vec3 distance) {
	viewDirection += distance;
	setViewMatrix();
}

void Camera::setViewMatrix() {
	glm::vec3 forward = glm::normalize(viewDirection);
	glm::vec3 up = glm::normalize(upDirection);
	glm::vec3 right = glm::normalize(glm::cross(up, -forward));

	glm::mat4 rotation = glm::mat4(1.0f);
	rotation[0] = glm::vec4(right, 0.0f);
	rotation[1] = glm::vec4(up, 0.0f);
	rotation[2] = glm::vec4(-forward, 0.0f);

	glm::mat4 translation = glm::mat4(1.0f);
	translation[3] = glm::vec4(-camPosition, 1.0f);

	viewMatrix = rotation * translation;
}
