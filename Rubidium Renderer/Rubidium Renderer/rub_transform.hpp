#pragma once

#include "glm/glm.hpp"

namespace rub
{
	class Transform
	{
	public:
		Transform();
		Transform(glm::vec3 position, glm::vec3 rotation);
		~Transform();

		glm::mat4 getMatrix();
		void rotate(glm::vec3 rotation);
		void move(glm::vec3 movement);

		glm::vec3 position;
		glm::vec3 rotation;
	private:
		glm::vec3 previousPosition = glm::vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
		glm::vec3 previousRotation = glm::vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

		glm::mat4 positionMatrix = glm::mat4{ 1.0f };
		glm::mat4 finalMatrix = glm::mat4{ 1.0f };
	};
}