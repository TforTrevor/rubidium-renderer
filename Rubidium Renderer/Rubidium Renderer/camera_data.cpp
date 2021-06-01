#include "camera_data.hpp"

namespace rub
{
	CameraData::CameraData(RubDevice& device, std::unique_ptr<RubSwapChain>& swapChain, VkDescriptorSetLayout& globalSetLayout) : device{ device }, globalSetLayout{ globalSetLayout }
	{
		recreateUniformBuffers(swapChain);
	}

	void CameraData::recreateUniformBuffers(std::unique_ptr<RubSwapChain>& swapChain)
	{
		cleanup();

		allocatedBuffers.resize(swapChain->MAX_FRAMES_IN_FLIGHT);
		descriptorSets.resize(swapChain->MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < swapChain->MAX_FRAMES_IN_FLIGHT; i++)
		{
			device.createBuffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, allocatedBuffers[i]);

			device.getDescriptor(globalSetLayout, descriptorSets[i]);

			VkDescriptorBufferInfo bufferInfo;
			bufferInfo.buffer = allocatedBuffers[i].buffer;
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
		vmaMapMemory(device.getAllocator(), allocatedBuffers[currentFrame].allocation, &data);
		memcpy(data, &cameraData, sizeof(GPUCameraData));
		vmaUnmapMemory(device.getAllocator(), allocatedBuffers[currentFrame].allocation);
	}

	void CameraData::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
		//Increment frame count when new command buffer is passed in (meaning new frame has started)
		if (currentBuffer != commandBuffer)
		{
			currentBuffer = commandBuffer;
			currentFrame = (currentFrame + 1) % allocatedBuffers.size();
		}		
	}

	void CameraData::cleanup()
	{
		for (int i = 0; i < allocatedBuffers.size(); i++)
		{
			vmaDestroyBuffer(device.getAllocator(), allocatedBuffers[i].buffer, allocatedBuffers[i].allocation);
		}
	}

	CameraData::~CameraData()
	{
		cleanup();
	}
}