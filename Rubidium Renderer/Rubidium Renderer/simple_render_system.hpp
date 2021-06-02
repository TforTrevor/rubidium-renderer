#pragma once

#include "rub_pipeline.hpp"
#include "rub_device.hpp"
#include "rub_game_object.hpp"
#include "rub_swap_chain.hpp"
#include "rub_renderer.hpp"

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
		SimpleRenderSystem(RubDevice& device, VkRenderPass renderPass, std::unique_ptr<GlobalDescriptor>& globalDescriptor);
		~SimpleRenderSystem();

		void renderModels(VkCommandBuffer commandBuffer, std::vector<RubGameObject> gameObjects, std::unique_ptr<GlobalDescriptor>& globalDescriptor);

	private:
		RubDevice& rubDevice;

		std::unique_ptr<RubPipeline> rubPipeline;
		VkPipelineLayout pipelineLayout;

		void createPipelineLayout(VkDescriptorSetLayout& setLayout);
		void createPipeline(VkRenderPass renderPass);

		int frameNumber = 0;
	};
}