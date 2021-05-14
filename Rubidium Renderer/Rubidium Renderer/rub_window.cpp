#include "rub_window.hpp"

#include <stdexcept>

namespace rub
{
	RubWindow::RubWindow(int width, int height, std::string name) : width{ width }, height{ height }, windowName{ name }
	{
		initWindow();
	}

	void RubWindow::initWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
	}

	void RubWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	RubWindow::~RubWindow()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}