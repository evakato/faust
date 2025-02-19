#pragma once

#include <iostream>

#include "math.hpp"
#include "state.hpp"

class Camera {
public:
	Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 up);

	void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
	void setPerspectiveProjection(float fovy, float aspect, float near, float far);


	const glm::mat4& getProjection() const { return projectionMatrix; }
	const glm::mat4& getView() const {
		return glm::lookAt(position, position + front, up);
	}
	const glm::mat4 getInvView() const {
		return glm::inverse(getView());
	}

	void translate(glm::vec3 offset);
	void rotate(float yawOffset, float pitchOffset);

private:
	void updateVectors();

	glm::mat4 projectionMatrix{ 1.f };

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	float yaw, pitch;

};