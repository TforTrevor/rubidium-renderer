#pragma once

#include "rub_device.hpp"
#include "rub_swap_chain.hpp"

#include <glm/glm.hpp>

namespace rub
{
	struct GPUCameraData
	{
		glm::mat4 view;
		glm::mat4 projection;
	};

	class CameraData
	{
	public:
		CameraData(RubDevice& device, std::unique_ptr<RubSwapChain>& swapChain, VkDescriptorSetLayout& globalSetLayout);
		~CameraData();

		void recreateUniformBuffers(std::unique_ptr<RubSwapChain>& swapChain);
		void updateBuffers(GPUCameraData cameraData);
		void bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
		void cleanup();

	private:
		RubDevice& device;
		VkDescriptorSetLayout& globalSetLayout;

		std::vector<AllocatedBuffer> allocatedBuffers;
		std::vector<VkDescriptorSet> descriptorSets;

		int currentFrame = 0;
		VkCommandBuffer currentBuffer;
	};
}