#include "rub_material.hpp"

#include "vk_util.hpp"

namespace rub
{
	Material::Material(Device& device, std::shared_ptr<Texture> albedo, std::shared_ptr<Texture> normalMap, std::shared_ptr<Texture> maskMap) 
		: device{ device }, albedo{ albedo }, normalMap{ normalMap }, maskMap{ maskMap }
	{
		VkDescriptorSetLayoutBinding albedoBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
		VkDescriptorSetLayoutBinding normalBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		VkDescriptorSetLayoutBinding maskBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);

		VkDescriptorSetLayoutBinding textureBinding[] = { albedoBinding, normalBinding, maskBinding };

		VkDescriptorSetLayoutCreateInfo textureLayoutInfo = {};
		textureLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		textureLayoutInfo.pNext = nullptr;
		textureLayoutInfo.bindingCount = 3;
		textureLayoutInfo.flags = 0;
		textureLayoutInfo.pBindings = textureBinding;

		vkCreateDescriptorSetLayout(device.getDevice(), &textureLayoutInfo, nullptr, &textureSetLayout);		
	}

	void Material::setup(VkDescriptorSetLayout& globalLayout, VkDescriptorSetLayout& objectSetLayout, VkRenderPass renderPass)
	{
		createBuffers();
		createPipelineLayout(globalLayout, objectSetLayout);
		createPipeline(renderPass);
	}

	void Material::createBuffers()
	{
		VkSamplerCreateInfo samplerInfo = VkUtil::samplesCreateInfo(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
		device.getDescriptor(textureSetLayout, textureDescriptor);

		vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &albedoSampler);
		VkDescriptorImageInfo albedoInfo;
		albedoInfo.sampler = albedoSampler;
		albedoInfo.imageView = albedo->getImageView();
		albedoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		
		vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &normalSampler);
		VkDescriptorImageInfo normalInfo;
		normalInfo.sampler = normalSampler;
		normalInfo.imageView = normalMap->getImageView();
		normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &maskSampler);
		VkDescriptorImageInfo maskInfo;
		maskInfo.sampler = maskSampler;
		maskInfo.imageView = maskMap->getImageView();
		maskInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet albedoDescriptor = VkUtil::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureDescriptor, &albedoInfo, 0);
		VkWriteDescriptorSet normalDescriptor = VkUtil::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureDescriptor, &normalInfo, 1);
		VkWriteDescriptorSet maskDescriptor = VkUtil::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureDescriptor, &maskInfo, 2);
		VkWriteDescriptorSet textureDescriptors[] = { albedoDescriptor, normalDescriptor, maskDescriptor };

		vkUpdateDescriptorSets(device.getDevice(), 3, textureDescriptors, 0, nullptr);
	}

	void Material::createPipelineLayout(VkDescriptorSetLayout& globalLayout, VkDescriptorSetLayout& objectSetLayout)
	{
		VkDescriptorSetLayout setLayouts[] = { globalLayout, objectSetLayout, textureSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 3;
		pipelineLayoutInfo.pSetLayouts = setLayouts;
		vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	}

	void Material::createPipeline(VkRenderPass renderPass)
	{
		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		//pipelineConfig.descriptorSetLayout = descriptorSetLayout;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<Pipeline>(device, "shaders/shader.vert.spv", "shaders/shader.frag.spv", pipelineConfig);
	}

	void Material::bind(VkCommandBuffer commandBuffer)
	{
		pipeline->bind(commandBuffer);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &textureDescriptor, 0, nullptr);
	}

	Material::~Material()
	{
		vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device.getDevice(), textureSetLayout, nullptr);
		vkDestroySampler(device.getDevice(), albedoSampler, nullptr);
		vkDestroySampler(device.getDevice(), normalSampler, nullptr);
		vkDestroySampler(device.getDevice(), maskSampler, nullptr);
	}
}