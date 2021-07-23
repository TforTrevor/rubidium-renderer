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
		bool wasWindowResized() { return framebufferResized; };
		void resetWindowResizedFlag() { framebufferResized = false; };
		void changeTitleSuffix(std::string suffix);

	private:
		int width;
		int height;
		bool framebufferResized = false;
		const std::string WINDOW_TITLE;
		GLFWwindow* window;

		void initWindow();
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	};
}