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
			glm::vec4 position;
		};

		struct GPUSceneData
		{
			glm::vec4 ambientColor;
			glm::vec4 sunDirection;
			glm::vec4 sunColor;
		};

		struct GPULightData
		{
			glm::vec4 lightPositions[4];
			glm::vec4 lightColors[4];
		};

		GlobalDescriptor(Device& device, std::unique_ptr<SwapChain>& swapChain);
		~GlobalDescriptor();

		void updateCameraBuffer(GPUCameraData camera);
		void updateSceneBuffer(GPUSceneData scene);
		void updateLightBuffer(GPULightData light);
		void bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
		VkDescriptorSetLayout getLayout() { return setLayout; };

	private:
		Device& device;

		VkDescriptorSetLayout setLayout;
		AllocatedBuffer cameraBuffer;
		AllocatedBuffer sceneBuffer;
		AllocatedBuffer lightBuffer;
		std::vector<VkDescriptorSet> globalDescriptors;

		const int FRAME_COUNT;
		int frameIndex = 0;
		VkCommandBuffer currentBuffer;

		void createLayouts();
		void createBuffers();
	};
}