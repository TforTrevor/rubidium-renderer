#include "rub_window.hpp"

#include <stdexcept>

namespace rub
{
	RubWindow::RubWindow(int width, int height, std::string name) : width{ width }, height{ height }, WINDOW_TITLE{ name }
	{
		initWindow();
	}

	void RubWindow::initWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, WINDOW_TITLE.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void RubWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void RubWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto rubWindow = reinterpret_cast<RubWindow*>(glfwGetWindowUserPointer(window));
		rubWindow->framebufferResized = true;
		rubWindow->width = width;
		rubWindow->height = height;
	}

	void RubWindow::changeTitleSuffix(std::string suffix)
	{
		std::string title = WINDOW_TITLE + " - " + suffix;
		glfwSetWindowTitle(window, title.c_str());
	}

	RubWindow::~RubWindow()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}