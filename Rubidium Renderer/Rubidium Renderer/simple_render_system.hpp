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
		glm::mat4 modelMatrix;
		glm::vec4 objectPosition;
	};

	struct GPUObjectData
	{
		glm::mat4 modelMatrix;
		glm::vec4 albedo;
		glm::vec4 maskMap;
	};

	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(RubDevice& device, VkRenderPass renderPass, std::unique_ptr<GlobalDescriptor>& globalDescriptor, std::unique_ptr<RubSwapChain>& swapChain);
		~SimpleRenderSystem();

		void renderModels(VkCommandBuffer commandBuffer, std::vector<RubGameObject> gameObjects, std::unique_ptr<GlobalDescriptor>& globalDescriptor);

	private:
		RubDevice& device;

		std::unique_ptr<RubPipeline> rubPipeline;
		VkPipelineLayout pipelineLayout;

		VkDescriptorSetLayout objectSetLayout;
		std::vector<AllocatedBuffer> objectBuffers;
		std::vector<VkDescriptorSet> objectDescriptors;

		void createPipelineLayout(VkDescriptorSetLayout& setLayout);
		void createPipeline(VkRenderPass renderPass);
		void createDescriptorLayouts();
		void createBuffers();

		int frameIndex = 0;
		const int FRAME_COUNT = 2;
		const int MAX_OBJECTS = 10000;
	};
}