#include "rub_camera.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace rub
{
	Camera::Camera(Window& window, int fov)
	{
		if (glfwRawMouseMotionSupported())
		{
			glfwSetInputMode(window.getWindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}

		float aspectRatio = (float)window.getExtent().width / (float)window.getExtent().height;
		projectionMatrix = glm::perspective((float)fov, aspectRatio, 0.3f, 1000.0f);
	}

	void Camera::updatePosition(Window& windowWrapper)
	{
		GLFWwindow* window = windowWrapper.getWindow();
		float moveSpeed = 0;

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			moveSpeed = 0.25f;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			moveSpeed = 0.05f;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
		{
			moveSpeed = 0.1f;
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			position += forward * moveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			position -= forward * moveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			position += right * moveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			position -= right * moveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			position += glm::vec3(0, 1, 0) * moveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			position -= glm::vec3(0, 1, 0) * moveSpeed;
		}
	}

	void Camera::updateRotation(Window& window)
	{
		if (!window.getCursorToggle())
		{
			double cursorX, cursorY;
			glfwGetCursorPos(window.getWindow(), &cursorX, &cursorY);
			cursorX /= window.getExtent().width;
			cursorY /= window.getExtent().height;

			double cursorDeltaX = 0;
			double cursorDeltaY = 0;
			if (previousCursorX != std::numeric_limits<double>::min() && previousCursorY != std::numeric_limits<double>::min())
			{
				cursorDeltaX = cursorX - previousCursorX;
				cursorDeltaY = cursorY - previousCursorY;
			}
			previousCursorX = cursorX;
			previousCursorY = cursorY;

			float sensitivity = 200.0f;

			pitch += cursorDeltaY * sensitivity;
			yaw += cursorDeltaX * sensitivity;
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;

			updateVectors();
		}		
	}

	void Camera::updateVectors()
	{
		forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		forward.y = sin(glm::radians(pitch));
		forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

		up = glm::normalize(glm::cross(right, forward));
	}

	glm::mat4 Camera::getProjectionMatrix()
	{
		return projectionMatrix;
	}

	glm::mat4 Camera::getViewMatrix()
	{
		return glm::lookAt(position, position + forward, up);
	}


	Camera::~Camera()
	{

	}
}