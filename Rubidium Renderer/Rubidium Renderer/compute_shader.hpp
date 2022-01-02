#pragma once

#include "device.hpp"
#include "pipeline.hpp"
#include <memory>

namespace rub
{
	class ComputeShader
	{
	public:
		ComputeShader(Device& device, const std::string& compPath, VkImageView targetImage, std::vector<VkDescriptorSetLayout>& setLayouts);
		~ComputeShader();

		void bind(VkCommandBuffer commandBuffer);

	private:
		Device& device;
		std::string compPath;
		VkImageView targetImage;

		VkDescriptorSetLayout setLayout;
		VkDescriptorSet descriptorSet;

		VkSampler targetSampler;

		VkPipelineLayout pipelineLayout;
		std::unique_ptr<Pipeline> pipeline;

		void createDescriptorSetLayout();
		void createBuffers();
		void createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts);
		void createPipeline();
	};
}