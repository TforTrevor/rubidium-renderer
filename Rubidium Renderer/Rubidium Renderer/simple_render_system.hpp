#pragma once

#include "rub_pipeline.hpp"
#include "rub_device.hpp"
#include "rub_render_object.hpp"
#include "rub_swap_chain.hpp"
#include "rub_renderer.hpp"
#include "rub_texture.hpp"

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
		SimpleRenderSystem(Device& device, std::unique_ptr<SwapChain>& swapChain);
		~SimpleRenderSystem();

		void renderModels(VkCommandBuffer commandBuffer, std::vector<RenderObject>& renderObjects, std::unique_ptr<GlobalDescriptor>& globalDescriptor);

	private:
		Device& device;

		VkDescriptorSetLayout objectLayout;
		std::vector<AllocatedBuffer> objectBuffers;
		std::vector<VkDescriptorSet> objectDescriptors;

		void createDescriptorLayouts();
		void createBuffers();

		size_t frameIndex = 0;
		const int FRAME_COUNT = 2;
		const int MAX_OBJECTS = 10000;
	};
}