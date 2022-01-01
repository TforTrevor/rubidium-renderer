#include "rub_compute_shader.hpp"

#include "vk_util.hpp"

namespace rub
{
	ComputeShader::ComputeShader(Device& device, const std::string& compPath, VkImageView targetImage, std::vector<VkDescriptorSetLayout>& setLayouts) 
		: device{ device }, compPath{ compPath }, targetImage{ targetImage }
	{
		createDescriptorSetLayout();
		createBuffers();
		createPipelineLayout(setLayouts);
		createPipeline();
	}

	void ComputeShader::createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding targetBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0);
		std::vector<VkDescriptorSetLayoutBinding> bindings = { targetBinding };

		VkDescriptorSetLayoutCreateInfo textureLayoutInfo{};
		textureLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		textureLayoutInfo.pNext = nullptr;
		textureLayoutInfo.bindingCount = bindings.size();
		textureLayoutInfo.flags = 0;
		textureLayoutInfo.pBindings = bindings.data();

		vkCreateDescriptorSetLayout(device.getDevice(), &textureLayoutInfo, nullptr, &setLayout);
	}

	void ComputeShader::createBuffers()
	{
		device.getDescriptor(setLayout, descriptorSet);

		VkSamplerCreateInfo samplerInfo = VkUtil::samplesCreateInfo(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 1);
		vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &targetSampler);

		VkDescriptorImageInfo info{};
		info.sampler = targetSampler;
		info.imageView = targetImage;
		info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet writeDescriptor = VkUtil::writeDescriptorImage(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptorSet, &info, 0);

		vkUpdateDescriptorSets(device.getDevice(), 1, &writeDescriptor, 0, nullptr);
	}

	void ComputeShader::createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts)
	{
		std::vector<VkDescriptorSetLayout> finalSet;
		finalSet.push_back(setLayout);
		finalSet.insert(finalSet.end(), setLayouts.begin(), setLayouts.end());

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = finalSet.size();
		pipelineLayoutInfo.pSetLayouts = finalSet.data();
		vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	}

	void ComputeShader::createPipeline()
	{
		pipeline = std::make_unique<Pipeline>(device, compPath, pipelineLayout);
	}

	void ComputeShader::bind(VkCommandBuffer commandBuffer)
	{
		pipeline->bind(commandBuffer);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
	}

	ComputeShader::~ComputeShader()
	{
		vkDestroySampler(device.getDevice(), targetSampler, nullptr);
		vkDestroyDescriptorSetLayout(device.getDevice(), setLayout, nullptr);
		vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
	}
};