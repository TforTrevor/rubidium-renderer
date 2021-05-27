#include "simple_render_system.hpp"

#include "camera_data.hpp"

#include <stdexcept>
#include <array>
#include <glm\ext\matrix_transform.hpp>
#include <glm\ext\matrix_clip_space.hpp>

namespace rub
{
	SimpleRenderSystem::SimpleRenderSystem(RubDevice& device, std::shared_ptr<RubSwapChain>& swapChain, VkRenderPass renderPass, GlobalDescriptor& globalDescriptor) 
		: rubDevice{ device }, rubSwapChain{ swapChain }, globalDescriptor{ globalDescriptor }
	{
		createPipelineLayout();
		createPipeline(renderPass);
	}

	void SimpleRenderSystem::createPipelineLayout()
	{
		VkPushConstantRange pushConstant{};
		pushConstant.offset = 0;
		pushConstant.size = sizeof(MeshPushConstants);
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &globalDescriptor.globalSetLayout;
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

	void SimpleRenderSystem::renderModels(VkCommandBuffer commandBuffer, std::vector<RubGameObject> gameObjects, std::shared_ptr<CameraData> cameraData)
	{
		rubPipeline->bind(commandBuffer);
		cameraData->bind(commandBuffer, pipelineLayout);
		for (RubGameObject object : gameObjects)
		{
			//update uniform buffers
			//make a model view matrix for rendering the object
			
			//model rotation
			glm::mat4 model = glm::rotate(glm::mat4{ 1.0f }, glm::radians(frameNumber * 0.4f), glm::vec3(0, 1, 0));

			MeshPushConstants constants;
			constants.render_matrix = model;

			//upload the matrix to the GPU via pushconstants
			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);
			object.model->bind(commandBuffer);
			object.model->draw(commandBuffer);
		}
		frameNumber++;
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(rubDevice.getDevice(), pipelineLayout, nullptr);
	}
}