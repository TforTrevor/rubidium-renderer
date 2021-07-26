#include "simple_render_system.hpp"

#include "vk_util.hpp"

#include <glm\ext\matrix_transform.hpp>
#include <glm\ext\matrix_clip_space.hpp>
#include <glm\gtx\euler_angles.hpp>

#include <stdexcept>
#include <array>
#include <chrono>

namespace rub
{
	SimpleRenderSystem::SimpleRenderSystem(Device& device,  std::unique_ptr<SwapChain>& swapChain) 
		: device{ device }, FRAME_COUNT{ swapChain->MAX_FRAMES_IN_FLIGHT }
	{
		createDescriptorLayouts();
		createBuffers();
	}

	void SimpleRenderSystem::createDescriptorLayouts()
	{
		VkDescriptorSetLayoutBinding objectBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

		VkDescriptorSetLayoutCreateInfo objectLayoutInfo = {};
		objectLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		objectLayoutInfo.pNext = nullptr;
		objectLayoutInfo.bindingCount = 1;
		objectLayoutInfo.flags = 0;
		objectLayoutInfo.pBindings = &objectBinding;

		vkCreateDescriptorSetLayout(device.getDevice(), &objectLayoutInfo, nullptr, &objectLayout);
	}

	void SimpleRenderSystem::createBuffers()
	{
		objectDescriptors.resize(FRAME_COUNT);
		objectBuffers.resize(FRAME_COUNT);

		for (int i = 0; i < FRAME_COUNT; i++)
		{
			//size_t bufferSize = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUObjectData)) * MAX_OBJECTS;
			size_t bufferSize = sizeof(GPUObjectData) * MAX_OBJECTS;
			device.createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, objectBuffers[i]);
			device.getDescriptor(objectLayout, objectDescriptors[i]);

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = objectBuffers[i].buffer;
			bufferInfo.range = bufferSize;

			VkWriteDescriptorSet setWrite = VkUtil::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, objectDescriptors[i], &bufferInfo, 0);
			vkUpdateDescriptorSets(device.getDevice(), 1, &setWrite, 0, nullptr);
		}
	}

	void SimpleRenderSystem::renderModels(VkCommandBuffer commandBuffer, std::vector<RenderObject>& renderObjects, std::unique_ptr<GlobalDescriptor>& globalDescriptor, VkRenderPass& renderPass)
	{
		VkDescriptorSetLayout globalLayout = globalDescriptor->getLayout();

		void* objectData;
		vmaMapMemory(device.getAllocator(), objectBuffers[frameIndex % FRAME_COUNT].allocation, &objectData);
		GPUObjectData* objectSSBO = (GPUObjectData*)objectData;

		for (int i = 0; i < renderObjects.size(); i++)
		{
			auto& object = renderObjects[i];

			//object.transform.rotate(glm::vec3(0, 0.4f, 0));

			//model = glm::rotate(model, glm::radians(frameIndex * 0.4f), glm::vec3(0, 1, 0));

			objectSSBO[i].modelMatrix = object.transform.getMatrix();
		}

		vmaUnmapMemory(device.getAllocator(), objectBuffers[frameIndex % FRAME_COUNT].allocation);

		std::shared_ptr<Material> previousMaterial = nullptr;
		std::shared_ptr<Model> previousModel = nullptr;

		for (int i = 0; i < renderObjects.size(); i++)
		{
			auto& object = renderObjects[i];

			if (object.material != previousMaterial)
			{
				if (!object.material->isReady())
				{
					object.material->setup(globalLayout, objectLayout, renderPass);
				}
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->getLayout(), 1, 1, &objectDescriptors[frameIndex % FRAME_COUNT], 0, nullptr);
				globalDescriptor->bind(commandBuffer, object.material->getLayout());
				object.material->bind(commandBuffer);
				previousMaterial = object.material;
			}

			if (object.model != previousModel)
			{
				object.model->bind(commandBuffer);
				previousModel = object.model;
			}

			object.model->draw(commandBuffer, i);
		}

		frameIndex++;
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyDescriptorSetLayout(device.getDevice(), objectLayout, nullptr);

		for (int i = 0; i < objectBuffers.size(); i++)
		{
			vmaDestroyBuffer(device.getAllocator(), objectBuffers[i].buffer, objectBuffers[i].allocation);
		}		
	}
}