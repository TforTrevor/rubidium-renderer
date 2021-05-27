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
		CameraData(RubDevice& device, std::shared_ptr<RubSwapChain>& swapChain, VkDescriptorSetLayout& globalSetLayout);
		~CameraData();

		void updateBuffers(GPUCameraData cameraData);
		void bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

	private:
		RubDevice& device;
		std::shared_ptr<RubSwapChain>& swapChain;
		VkDescriptorSetLayout& globalSetLayout;

		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBufferMemory;
		std::vector<VkDescriptorSet> descriptorSets;

		int currentFrame = 0;

		void createUniformBuffers();
	};
}