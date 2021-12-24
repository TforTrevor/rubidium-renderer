#pragma once

#include "rub_device.hpp"
#include "rub_texture.hpp"
#include "rub_pipeline.hpp"

#include <memory>

namespace rub
{
	class Material
	{
	public:
		Material(Device& device, std::shared_ptr<Texture> albedo, std::shared_ptr<Texture> normalMap, std::shared_ptr<Texture> maskMap);
		~Material();

		bool isReady() { return pipelineLayout != nullptr && pipeline != nullptr; };
		void setup(VkDescriptorSetLayout& globalLayout, VkDescriptorSetLayout& objectSetLayout);
		void bind(VkCommandBuffer commandBuffer);
		VkPipelineLayout getLayout() { return pipelineLayout; };
		
	private:
		void createBuffers();
		void createPipelineLayout(VkDescriptorSetLayout& globalLayout, VkDescriptorSetLayout& objectSetLayout);
		void createPipeline();

		Device& device;

		std::shared_ptr<Texture> albedo;
		std::shared_ptr<Texture> normalMap;
		std::shared_ptr<Texture> maskMap;

		VkDescriptorSetLayout textureSetLayout;
		VkDescriptorSet textureDescriptor;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;

		VkSampler albedoSampler;
		VkSampler normalSampler;
		VkSampler maskSampler;
	};
}