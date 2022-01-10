#pragma once

#include "transform.hpp"
#include "window.hpp"

namespace rub
{
	class Camera
	{
	public:
		Camera(Window& window, int fov);
		~Camera();

		void updatePosition(Window& window);
		void updateRotation(Window& window);
		glm::mat4 getProjectionMatrix();
		glm::mat4 getViewMatrix();

		void setPosition(glm::vec3 pos) { position = pos; };
		glm::vec3 getPosition() { return position; };

	private:
		float pitch = 0;
		float yaw = 90;

		glm::vec3 position = glm::vec3(0, 0, 0);
		glm::vec3 forward = glm::vec3(0, 0, 1);
		glm::vec3 right = glm::vec3(1, 0, 0);
		glm::vec3 up = glm::vec3(0, 1, 0);

		double previousCursorX = std::numeric_limits<double>::min();
		double previousCursorY = std::numeric_limits<double>::min();

		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;

		void updateVectors();
	};
}