#pragma once

#include "rub_window.hpp"
#include "rub_device.hpp"
#include "rub_swap_chain.hpp"

#include "global_descriptor.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace rub
{
	class RubRenderer
	{
	public:
		RubRenderer(RubWindow& window, RubDevice& device);
		~RubRenderer();

		VkRenderPass getRenderPass() const { return rubSwapChain->getRenderPass(); }
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
		//std::shared_ptr<RubSwapChain>& getSwapChain() { return rubSwapChain; };
		std::unique_ptr<GlobalDescriptor>& getGlobalDescriptor() { return globalDescriptor; };

	private:
		RubWindow& rubWindow;
		RubDevice& rubDevice;

		std::unique_ptr<RubSwapChain> rubSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;
		bool isFrameStarted = false;
		uint32_t currentImageIndex;
		
		//GlobalDescriptor globalDescriptor;
		std::unique_ptr<GlobalDescriptor> globalDescriptor;

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
		void createGlobalDescriptor();
		void updateCamera();
	};
}