#pragma once

#include <Volk/volk.h>
#include <GLFW/glfw3.h>

#include <string>

namespace rub
{
	class Window
	{
	public:
		Window(int width, int height, std::string name);
		~Window();
		bool shouldClose() { return glfwWindowShouldClose(window); }
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; };
		bool wasWindowResized() { return framebufferResized; };
		void resetWindowResizedFlag() { framebufferResized = false; };
		void changeTitleSuffix(std::string suffix);
		GLFWwindow* getWindow() { return window; };
		bool getCursorToggle();

	private:
		int width;
		int height;
		bool framebufferResized = false;
		const std::string WINDOW_TITLE;
		GLFWwindow* window;

		void initWindow();
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		static void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods);
	};
}