#include "camera_data.hpp"

namespace rub
{
	CameraData::CameraData(RubDevice& device, std::shared_ptr<RubSwapChain>& swapChain, VkDescriptorSetLayout& globalSetLayout) 
		: device{ device }, swapChain{ swapChain }, globalSetLayout{ globalSetLayout }
	{
		createUniformBuffers();
	}

	void CameraData::createUniformBuffers()
	{
		uniformBuffers.resize(swapChain->MAX_FRAMES_IN_FLIGHT);
		uniformBufferMemory.resize(swapChain->MAX_FRAMES_IN_FLIGHT);
		descriptorSets.resize(swapChain->MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < swapChain->MAX_FRAMES_IN_FLIGHT; i++)
		{
			device.createBuffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				uniformBuffers[i], uniformBufferMemory[i]);

			device.getDescriptor(globalSetLayout, descriptorSets[i]);

			VkDescriptorBufferInfo bufferInfo;
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(GPUCameraData);

			VkWriteDescriptorSet setWrite = {};
			setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			setWrite.pNext = nullptr;
			setWrite.dstBinding = 0;
			setWrite.dstSet = descriptorSets[i];
			setWrite.descriptorCount = 1;
			setWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			setWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(device.getDevice(), 1, &setWrite, 0, nullptr);
		}
	}

	void CameraData::updateBuffers(GPUCameraData cameraData)
	{
		void* data;
		vkMapMemory(device.getDevice(), uniformBufferMemory[currentFrame], 0, sizeof(uniformBuffers[currentFrame]), 0, &data);
		memcpy(data, &cameraData, sizeof(GPUCameraData));
		vkUnmapMemory(device.getDevice(), uniformBufferMemory[currentFrame]);
	}

	void CameraData::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
		currentFrame = (currentFrame + 1) % uniformBuffers.size();
	}

	CameraData::~CameraData()
	{
		for (int i = 0; i < uniformBuffers.size(); i++)
		{
			vkDestroyBuffer(device.getDevice(), uniformBuffers[i], nullptr);
			vkFreeMemory(device.getDevice(), uniformBufferMemory[i], nullptr);
		}
	}
}