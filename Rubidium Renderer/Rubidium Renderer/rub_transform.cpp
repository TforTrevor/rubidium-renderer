#include "rub_transform.hpp"

#include <glm\ext\matrix_transform.hpp>
#include <glm\gtx\euler_angles.hpp>

namespace rub
{
	Transform::Transform() : position{ glm::vec3(0, 0, 0) }, rotation{ glm::vec3(0, 0, 0) }
	{

	}

	Transform::Transform(glm::vec3 position, glm::vec3 rotation) : position{ position }, rotation{ rotation }
	{

	}

	glm::mat4 Transform::getMatrix()
	{
		glm::mat4 matrix = glm::mat4{ 1.0f };
		if (position != previousPosition)
		{
			matrix = glm::translate(matrix, position);
			previousPosition = position;
			positionMatrix = matrix;
		}
		if (rotation != previousRotation)
		{
			glm::mat4 rotationMatrix = positionMatrix * glm::eulerAngleXYZ(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z));
			previousRotation = rotation;
			finalMatrix = rotationMatrix;
		}

		return finalMatrix;
	}

	void Transform::rotate(glm::vec3 rotate)
	{
		glm::vec3 add = glm::vec3(rotation.x + rotate.x, rotation.y + rotate.y, rotation.z + rotate.z);
		glm::vec3 modulo = glm::vec3(fmod(add.x, 360.0f), fmod(add.y, 360.0f), fmod(add.z, 360.0f));
		rotation = modulo;
	}

	Transform::~Transform()
	{

	}
}