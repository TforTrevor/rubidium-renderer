#include "rub_window.hpp"

#include <stdexcept>

namespace rub
{
	Window::Window(int width, int height, std::string name) : width{ width }, height{ height }, WINDOW_TITLE{ name }
	{
		initWindow();
	}

	void Window::initWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, WINDOW_TITLE.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto rubWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		rubWindow->framebufferResized = true;
		rubWindow->width = width;
		rubWindow->height = height;
	}

	void Window::changeTitleSuffix(std::string suffix)
	{
		std::string title = WINDOW_TITLE + " - " + suffix;
		glfwSetWindowTitle(window, title.c_str());
	}

	Window::~Window()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}