#include "rub_material.hpp"

#include "vk_util.hpp"

namespace rub
{
	Material::Material(Device& device, const std::string& vertexPath, const std::string& fragPath) : device{ device }, vertPath{ vertexPath }, fragPath{ fragPath }
	{

	}

	void Material::addTexture(std::shared_ptr<Texture> texture)
	{
		textures.push_back(texture);
	}

	void Material::setup(std::vector<VkDescriptorSetLayout>& setLayouts, VkRenderPass renderPass)
	{
		createDescriptorSetLayout();
		createPipelineLayout(setLayouts);
		createPipeline(renderPass);
	}

	void Material::createDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> textureBindings;
		textureBindings.resize(textures.size());
		for (int i = 0; i < textures.size(); i++)
		{
			textureBindings[i] = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, i);
		}

		VkDescriptorSetLayoutCreateInfo textureLayoutInfo = {};
		textureLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		textureLayoutInfo.pNext = nullptr;
		textureLayoutInfo.bindingCount = textureBindings.size();
		textureLayoutInfo.flags = 0;
		textureLayoutInfo.pBindings = textureBindings.data();

		vkCreateDescriptorSetLayout(device.getDevice(), &textureLayoutInfo, nullptr, &textureSetLayout);

		createBuffers();
	}

	void Material::createBuffers()
	{
		VkSamplerCreateInfo samplerInfo = VkUtil::samplesCreateInfo(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
		device.getDescriptor(textureSetLayout, textureDescriptor);

		std::vector<VkWriteDescriptorSet> descriptors;
		descriptors.resize(textures.size());
		textureSamplers.resize(textures.size());
		for (int i = 0; i < textures.size(); i++)
		{
			vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &textureSamplers[i]);
			VkDescriptorImageInfo info{};
			info.sampler = textureSamplers[i];
			info.imageView = textures[i]->getImageView();
			info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			descriptors[i] = VkUtil::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureDescriptor, &info, i);
		}

		vkUpdateDescriptorSets(device.getDevice(), textures.size(), descriptors.data(), 0, nullptr);
	}

	void Material::createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts)
	{
		std::vector<VkDescriptorSetLayout> finalSet;
		finalSet.insert(finalSet.end(), setLayouts.begin(), setLayouts.end());
		finalSet.push_back(textureSetLayout);
		descriptorSetCount = finalSet.size();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = finalSet.size();
		pipelineLayoutInfo.pSetLayouts = finalSet.data();
		vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	}

	void Material::createPipeline(VkRenderPass renderPass)
	{
		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		//pipelineConfig.descriptorSetLayout = descriptorSetLayout;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipelineConfig.depthStencilInfo.depthCompareOp = depthCompareOp;
		pipeline = std::make_unique<Pipeline>(device, vertPath, fragPath, pipelineConfig);
	}

	void Material::bind(VkCommandBuffer commandBuffer)
	{
		pipeline->bind(commandBuffer);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, descriptorSetCount - 1, 1, &textureDescriptor, 0, nullptr);
	}

	Material::~Material()
	{
		for (VkSampler& sampler : textureSamplers)
		{
			vkDestroySampler(device.getDevice(), sampler, nullptr);
		}
		vkDestroyDescriptorSetLayout(device.getDevice(), textureSetLayout, nullptr);
		if (isReady())
			vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
	}
}