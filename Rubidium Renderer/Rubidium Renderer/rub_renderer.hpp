#pragma once

#include "rub_window.hpp"
#include "rub_device.hpp"
#include "rub_swap_chain.hpp"
#include "rub_camera.hpp"

#include "global_descriptor.hpp"

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

		//VkRenderPass getRenderPass() const { return swapChain->getRenderPass(); }
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
		std::unique_ptr<GlobalDescriptor>& getGlobalDescriptor() { return globalDescriptor; };
		void setCamera(std::shared_ptr<Camera> newCamera) { camera = newCamera; };

	private:
		Window& window;
		Device& device;

		std::unique_ptr<SwapChain> swapChain;
		std::vector<VkCommandBuffer> commandBuffers;
		bool isFrameStarted = false;
		uint32_t currentImageIndex;
		
		//GlobalDescriptor globalDescriptor;
		std::unique_ptr<GlobalDescriptor> globalDescriptor;
		std::shared_ptr<Camera> camera;

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
		void updateCamera();
	};
}