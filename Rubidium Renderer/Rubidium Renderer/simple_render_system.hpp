#pragma once

#include "rub_pipeline.hpp"
#include "rub_device.hpp"
#include "rub_game_object.hpp"

#include <memory>
#include <vector>

namespace rub
{
	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(RubDevice& device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		void renderModels(VkCommandBuffer commandBuffer, std::vector<RubGameObject> gameObjects);

	private:
		RubDevice& rubDevice;

		std::unique_ptr<RubPipeline> rubPipeline;
		VkPipelineLayout pipelineLayout;

		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);
	};
}