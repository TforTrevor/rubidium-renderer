#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace rub
{
	class RubWindow
	{
	public:
		RubWindow(int width, int height, std::string name);
		~RubWindow();
		bool shouldClose() { return glfwWindowShouldClose(window); }
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; };

	private:
		const int width;
		const int height;
		std::string windowName;
		GLFWwindow* window;

		void initWindow();
	};
}