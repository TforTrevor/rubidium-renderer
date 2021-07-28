#include "global_descriptor.hpp"

#include "vk_util.hpp"

namespace rub
{
	GlobalDescriptor::GlobalDescriptor(Device& device, std::unique_ptr<SwapChain>& swapChain) : device{ device }, FRAME_COUNT{ swapChain->MAX_FRAMES_IN_FLIGHT }
	{
		globalDescriptors.resize(FRAME_COUNT);

		createLayouts();
		createBuffers();
	}

	void GlobalDescriptor::createLayouts()
	{
		VkDescriptorSetLayoutBinding cameraBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);
		VkDescriptorSetLayoutBinding sceneBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		VkDescriptorSetLayoutBinding lightBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
		VkDescriptorSetLayoutBinding hdriBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);
		VkDescriptorSetLayoutBinding bindings[] = { cameraBinding, sceneBinding, lightBinding, hdriBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.bindingCount = 4;
		layoutInfo.flags = 0;
		layoutInfo.pBindings = bindings;

		vkCreateDescriptorSetLayout(device.getDevice(), &layoutInfo, nullptr, &setLayout);
	}

	void GlobalDescriptor::createBuffers()
	{
		size_t cameraSize = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUCameraData)) * FRAME_COUNT;
		device.createBuffer(cameraSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, cameraBuffer);

		size_t sceneSize = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUSceneData)) * FRAME_COUNT;
		device.createBuffer(sceneSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, sceneBuffer);

		size_t lightSize = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPULightData)) * FRAME_COUNT;
		device.createBuffer(lightSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, lightBuffer);

		//VkSamplerCreateInfo samplerInfo = VkUtil::samplesCreateInfo(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		//vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &hdriSampler);
		//VkDescriptorImageInfo hdriInfo;
		//hdriInfo.sampler = hdriSampler;
		//hdriInfo.imageView = albedo->getImageView();
		//hdriInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		for (int i = 0; i < FRAME_COUNT; i++)
		{
			device.getDescriptor(setLayout, globalDescriptors[i]);

			VkDescriptorBufferInfo cameraBufferInfo{};
			cameraBufferInfo.buffer = cameraBuffer.buffer;
			cameraBufferInfo.range = sizeof(GPUCameraData);

			VkDescriptorBufferInfo sceneBufferInfo{};
			sceneBufferInfo.buffer = sceneBuffer.buffer;
			sceneBufferInfo.range = sizeof(GPUSceneData);

			VkDescriptorBufferInfo lightBufferInfo{};
			lightBufferInfo.buffer = lightBuffer.buffer;
			lightBufferInfo.range = sizeof(GPULightData);

			VkWriteDescriptorSet cameraWrite = VkUtil::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, globalDescriptors[i], &cameraBufferInfo, 0);
			VkWriteDescriptorSet sceneWrite = VkUtil::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, globalDescriptors[i], &sceneBufferInfo, 1);
			VkWriteDescriptorSet lightWrite = VkUtil::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, globalDescriptors[i], &lightBufferInfo, 2);
			//VkWriteDescriptorSet hdriWrite = VkUtil::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, globalDescriptors[i], &hdriInfo, 3);
			VkWriteDescriptorSet setWrites[] = { cameraWrite, sceneWrite, lightWrite };

			vkUpdateDescriptorSets(device.getDevice(), 3, setWrites, 0, nullptr);
		}		
	}

	void GlobalDescriptor::updateCameraBuffer(GPUCameraData camera)
	{
		char* cameraData;
		vmaMapMemory(device.getAllocator(), cameraBuffer.allocation, (void**)&cameraData);
		cameraData += VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUCameraData)) * frameIndex;
		memcpy(cameraData, &camera, sizeof(GPUCameraData));
		vmaUnmapMemory(device.getAllocator(), cameraBuffer.allocation);
	}

	void GlobalDescriptor::updateSceneBuffer(GPUSceneData scene)
	{
		char* sceneData;
		vmaMapMemory(device.getAllocator(), sceneBuffer.allocation, (void**)&sceneData);
		sceneData += VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUSceneData)) * frameIndex;
		memcpy(sceneData, &scene, sizeof(GPUSceneData));
		vmaUnmapMemory(device.getAllocator(), sceneBuffer.allocation);
	}

	void GlobalDescriptor::updateLightBuffer(GPULightData light)
	{
		char* lightData;
		vmaMapMemory(device.getAllocator(), lightBuffer.allocation, (void**)& lightData);
		lightData += VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPULightData)) * frameIndex;
		memcpy(lightData, &light, sizeof(GPULightData));
		vmaUnmapMemory(device.getAllocator(), lightBuffer.allocation);
	}

	void GlobalDescriptor::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
	{
		uint32_t cameraOffset = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUCameraData)) * frameIndex;
		uint32_t sceneOffset = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUSceneData)) * frameIndex;
		uint32_t lightOffset = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPULightData)) * frameIndex;
		uint32_t offsets[] = { cameraOffset, sceneOffset, lightOffset };
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &globalDescriptors[frameIndex], 3, offsets);

		//Increment frame count when new command buffer is passed in (meaning new frame has started)
		if (currentBuffer != commandBuffer)
		{
			currentBuffer = commandBuffer;
			frameIndex = (frameIndex + 1) % FRAME_COUNT;
		}
	}

	GlobalDescriptor::~GlobalDescriptor()
	{
		vmaDestroyBuffer(device.getAllocator(), cameraBuffer.buffer, cameraBuffer.allocation);
		vmaDestroyBuffer(device.getAllocator(), sceneBuffer.buffer, sceneBuffer.allocation);
		vmaDestroyBuffer(device.getAllocator(), lightBuffer.buffer, lightBuffer.allocation);
		vkDestroyDescriptorSetLayout(device.getDevice(), setLayout, nullptr);
	}
}