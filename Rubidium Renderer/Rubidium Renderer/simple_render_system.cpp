#include "simple_render_system.hpp"

#include "vk_util.hpp"

#include <glm\ext\matrix_transform.hpp>
#include <glm\ext\matrix_clip_space.hpp>
#include <glm\gtx\euler_angles.hpp>

#include <stdexcept>
#include <array>

namespace rub
{
	SimpleRenderSystem::SimpleRenderSystem(RubDevice& device, VkRenderPass renderPass, std::unique_ptr<GlobalDescriptor>& globalDescriptor, std::unique_ptr<RubSwapChain>& swapChain) 
		: device{ device }, FRAME_COUNT{ swapChain->MAX_FRAMES_IN_FLIGHT }
	{
		VkDescriptorSetLayout setLayout = globalDescriptor->getLayout();
		createDescriptorLayouts();
		createBuffers();
		createPipelineLayout(setLayout);
		createPipeline(renderPass);
	}

	void SimpleRenderSystem::createDescriptorLayouts()
	{
		VkDescriptorSetLayoutBinding binding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.bindingCount = 1;
		layoutInfo.flags = 0;
		layoutInfo.pBindings = &binding;

		vkCreateDescriptorSetLayout(device.getDevice(), &layoutInfo, nullptr, &objectSetLayout);
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

			device.getDescriptor(objectSetLayout, objectDescriptors[i]);

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = objectBuffers[i].buffer;
			bufferInfo.range = bufferSize;

			VkWriteDescriptorSet setWrite = VkUtil::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, objectDescriptors[i], &bufferInfo, 0);

			vkUpdateDescriptorSets(device.getDevice(), 1, &setWrite, 0, nullptr);
		}
	}

	void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout& globalLayout)
	{
		//VkPushConstantRange pushConstant{};
		//pushConstant.offset = 0;
		//pushConstant.size = sizeof(MeshPushConstants);
		//pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayout setLayouts[] = { globalLayout, objectSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = setLayouts;
		//pipelineLayoutInfo.pushConstantRangeCount = 1;
		//pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
		if (vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		RubPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		//pipelineConfig.descriptorSetLayout = descriptorSetLayout;
		pipelineConfig.pipelineLayout = pipelineLayout;
		rubPipeline = std::make_unique<RubPipeline>(device, "shaders/shader.vert.spv", "shaders/shader.frag.spv", pipelineConfig);
	}

	void SimpleRenderSystem::renderModels(VkCommandBuffer commandBuffer, std::vector<RubGameObject> gameObjects, std::unique_ptr<GlobalDescriptor>& globalDescriptor)
	{
		rubPipeline->bind(commandBuffer);
		globalDescriptor->bind(commandBuffer, pipelineLayout);

		void* objectData;
		vmaMapMemory(device.getAllocator(), objectBuffers[frameIndex % FRAME_COUNT].allocation, &objectData);
		GPUObjectData* objectSSBO = (GPUObjectData*)objectData;

		for (int i = 0; i < gameObjects.size(); i++)
		{
			auto object = gameObjects[i];

			glm::mat4 model = glm::translate(glm::mat4{ 1.0f }, object.position);
			model *= glm::eulerAngleXYZ(object.rotation.x, object.rotation.y, object.rotation.z);
			model = glm::rotate(model, glm::radians(frameIndex * 0.4f), glm::vec3(0, 1, 0));

			objectSSBO[i].modelMatrix = model;
			objectSSBO[i].albedo = object.material.albedo;
			objectSSBO[i].maskMap = object.material.maskMap;
		}

		vmaUnmapMemory(device.getAllocator(), objectBuffers[frameIndex % FRAME_COUNT].allocation);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &objectDescriptors[frameIndex % FRAME_COUNT], 0, nullptr);

		for (int i = 0; i < gameObjects.size(); i++)
		{
			auto object = gameObjects[i];

			object.model->bind(commandBuffer);
			object.model->draw(commandBuffer, i);
		}

		frameIndex++;
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);

		vkDestroyDescriptorSetLayout(device.getDevice(), objectSetLayout, nullptr);

		for (int i = 0; i < objectBuffers.size(); i++)
		{
			vmaDestroyBuffer(device.getAllocator(), objectBuffers[i].buffer, objectBuffers[i].allocation);
		}		
	}
}