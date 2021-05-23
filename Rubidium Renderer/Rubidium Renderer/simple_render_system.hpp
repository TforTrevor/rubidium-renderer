#pragma once

#include "rub_pipeline.hpp"
#include "rub_device.hpp"
#include "rub_game_object.hpp"
#include "rub_swap_chain.hpp"

#include <memory>
#include <vector>

namespace rub
{
	struct MeshPushConstants
	{
		glm::mat4 render_matrix;
	};

	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(RubDevice& device, std::shared_ptr<RubSwapChain>& swapChain, VkRenderPass renderPass);
		~SimpleRenderSystem();

		void renderModels(VkCommandBuffer commandBuffer, std::vector<RubGameObject> gameObjects);

	private:
		RubDevice& rubDevice;
		std::shared_ptr<RubSwapChain>& rubSwapChain;

		std::unique_ptr<RubPipeline> rubPipeline;
		VkDescriptorSetLayout descriptorSetLayout;
		VkPipelineLayout pipelineLayout;

		void createDescriptorSetLayout();
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		int frameNumber = 0;
	};
}