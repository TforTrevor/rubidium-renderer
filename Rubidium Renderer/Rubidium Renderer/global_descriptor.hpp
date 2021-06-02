#pragma once

#include "rub_device.hpp"
#include "rub_swap_chain.hpp"

#include "glm/glm.hpp"

namespace rub
{
	class GlobalDescriptor
	{
	public:
		struct GPUCameraData
		{
			glm::mat4 view;
			glm::mat4 projection;
		};

		struct GPUSceneData
		{
			glm::vec4 ambientColor;
			glm::vec4 sunDirection;
			glm::vec4 sunColor;
		};

		GlobalDescriptor(RubDevice& device, std::unique_ptr<RubSwapChain>& swapChain);
		~GlobalDescriptor();

		void createLayouts();
		void createBuffers();
		void updateCameraBuffer(GPUCameraData camera);
		void updateSceneBuffer(GPUSceneData scene);
		void bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
		VkDescriptorSetLayout getLayout() { return setLayout; };

	private:
		RubDevice& device;

		VkDescriptorSetLayout setLayout;
		AllocatedBuffer cameraBuffer;
		AllocatedBuffer sceneBuffer;
		std::vector<VkDescriptorSet> globalDescriptors;

		const int FRAME_COUNT;
		int frameIndex = 0;
		VkCommandBuffer currentBuffer;
	};
}