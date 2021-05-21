#include "simple_render_system.hpp"

#include <stdexcept>
#include <array>

namespace rub
{
	SimpleRenderSystem::SimpleRenderSystem(RubDevice& device, VkRenderPass renderPass) : rubDevice{ device }
	{
		createPipelineLayout();
		createPipeline(renderPass);
	}

	void SimpleRenderSystem::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
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
			object.model->bind(commandBuffer);
			object.model->draw(commandBuffer);
		}
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		//vkDestroyDescriptorSetLayout(rubDevice.getDevice(), descriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(rubDevice.getDevice(), pipelineLayout, nullptr);
	}
}