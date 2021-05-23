#include "simple_render_system.hpp"

#include <stdexcept>
#include <array>
#include <glm\ext\matrix_transform.hpp>
#include <glm\ext\matrix_clip_space.hpp>

namespace rub
{
	SimpleRenderSystem::SimpleRenderSystem(RubDevice& device, std::shared_ptr<RubSwapChain>& swapChain, VkRenderPass renderPass) : rubDevice{ device }, rubSwapChain{ swapChain }
	{
		createPipelineLayout();
		createPipeline(renderPass);
	}

	void SimpleRenderSystem::createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(rubDevice.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void SimpleRenderSystem::createPipelineLayout()
	{
		VkPushConstantRange pushConstant{};
		pushConstant.offset = 0;
		pushConstant.size = sizeof(MeshPushConstants);
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
		if (vkCreatePipelineLayout(rubDevice.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
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
		rubPipeline = std::make_unique<RubPipeline>(rubDevice, "shaders/shader.vert.spv", "shaders/shader.frag.spv", pipelineConfig);
	}

	void SimpleRenderSystem::renderModels(VkCommandBuffer commandBuffer, std::vector<RubGameObject> gameObjects)
	{
		rubPipeline->bind(commandBuffer);
		for (RubGameObject object : gameObjects)
		{
			//update uniform buffers
					//make a model view matrix for rendering the object
		//camera position
			glm::vec3 camPos = { 0.f,0.f,-2.f };

			glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
			//camera projection
			glm::mat4 projection = glm::perspective(glm::radians(70.f), rubSwapChain->getExtentAspectRatio(), 0.1f, 200.0f);
			projection[1][1] *= -1;
			//model rotation
			glm::mat4 model = glm::rotate(glm::mat4{ 1.0f }, glm::radians(frameNumber * 0.4f), glm::vec3(0, 1, 0));

			//calculate final mesh matrix
			glm::mat4 mesh_matrix = projection * view * model;

			MeshPushConstants constants;
			constants.render_matrix = mesh_matrix;

			//upload the matrix to the GPU via pushconstants
			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);
			object.model->bind(commandBuffer);
			object.model->draw(commandBuffer);
		}
		frameNumber++;
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyDescriptorSetLayout(rubDevice.getDevice(), descriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(rubDevice.getDevice(), pipelineLayout, nullptr);
	}
}