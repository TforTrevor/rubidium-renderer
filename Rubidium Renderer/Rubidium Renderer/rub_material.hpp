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
		Material(Device& device, const std::string& vertexPath, const std::string& fragPath);
		~Material();

		void addTexture(std::shared_ptr<Texture> texture);
		bool isReady() { return pipelineLayout != nullptr && pipeline != nullptr; };
		void setup(std::vector<VkDescriptorSetLayout>& setLayouts, VkRenderPass renderPass);
		void bind(VkCommandBuffer commandBuffer);
		VkPipelineLayout getLayout() { return pipelineLayout; };
		
	private:
		void createBuffers();
		void createDescriptorSetLayout();
		void createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts);
		void createPipeline(VkRenderPass renderPass);

		Device& device;

		std::string vertPath;
		std::string fragPath;

		std::vector<std::shared_ptr<Texture>> textures;
		std::vector<VkSampler> textureSamplers;

		VkDescriptorSetLayout textureSetLayout;
		VkDescriptorSet textureDescriptor;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;
		int descriptorSetCount = 0;
	};
}