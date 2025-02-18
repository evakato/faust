#pragma once

#include <iostream>

#include "math.hpp"

class Camera {
public:
	void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
	void setPerspectiveProjection(float fovy, float aspect, float near, float far);


	const glm::mat4& getProjection() const { return projectionMatrix; }
	const glm::mat4& getView() const { return viewMatrix; }

	void setCamera(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0.f, 1.f, 0.f });
	void translate(glm::vec3 distance);
	void pan(glm::vec3 distance);

private:
	void setViewMatrix();

	glm::mat4 projectionMatrix{ 1.f };
	glm::mat4 viewMatrix{ 1.f };

	glm::vec3 camPosition;
	glm::vec3 viewDirection;
	glm::vec3 upDirection;
};