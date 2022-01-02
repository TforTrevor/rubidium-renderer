#pragma once

#include "rub_window.hpp"
#include "rub_device.hpp"
#include "rub_swap_chain.hpp"
#include "rub_camera.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace rub
{
	class Renderer
	{
	public:
		Renderer(Window& window, Device& device);
		~Renderer();

		VkRenderPass getRenderPass() const { return swapChain->getRenderPass(); }
		bool isFrameInProgress() const { return isFrameStarted; }
		VkCommandBuffer getCurrentCommandBuffer()
		{
			assert(isFrameStarted && "can't get command buffer when frame isn't started");
			return commandBuffers[currentImageIndex];
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginRenderPass(VkCommandBuffer commandBuffer);
		void endRenderPass(VkCommandBuffer commandBuffer);
		std::unique_ptr<SwapChain>& getSwapChain() { return swapChain; };

	private:
		Window& window;
		Device& device;

		std::unique_ptr<SwapChain> swapChain;
		std::vector<VkCommandBuffer> commandBuffers;
		bool isFrameStarted = false;
		uint32_t currentImageIndex;

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
	};
}