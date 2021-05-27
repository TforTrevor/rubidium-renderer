#pragma once

#include "rub_window.hpp"
#include "rub_device.hpp"
#include "rub_swap_chain.hpp"

#include "camera_data.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace rub
{
	struct GlobalDescriptor
	{
		VkDescriptorSetLayout globalSetLayout;
		VkDeviceMemory deviceMemory;
		VkDeviceSize deviceSize;
		void* mappedData;
	};

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
		std::shared_ptr<RubSwapChain>& getSwapChain() { return rubSwapChain; };
		GlobalDescriptor& getGlobalDescriptor() { return globalDescriptor; };
		std::shared_ptr<CameraData> getCameraData() { return cameraData; };

	private:
		RubWindow& rubWindow;
		RubDevice& rubDevice;

		std::shared_ptr<RubSwapChain> rubSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;
		bool isFrameStarted = false;
		uint32_t currentImageIndex;
		
		GlobalDescriptor globalDescriptor;
		std::shared_ptr<CameraData> cameraData;

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
		void createGlobalDescriptor();
		void updateCamera();
	};
}