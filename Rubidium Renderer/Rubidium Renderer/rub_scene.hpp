#pragma once

#include "rub_render_object.hpp"
#include "rub_cubemap.hpp"
#include "rub_camera.hpp"
#include "rub_swap_chain.hpp"
#include "rub_skybox.hpp"

namespace rub
{
	class Scene
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
			glm::vec4 sunDirection; //4th component for alignment
			glm::vec4 sunColor;
			glm::vec4 lightPositions[4]; //4th component for alignment
			glm::vec4 lightColors[4];
			uint32_t lightCount;
		};

		struct GPUObjectData
		{
			glm::mat4 modelMatrix;
			glm::mat4 MVP;
		};

		Scene(Device& device, std::unique_ptr<SwapChain>& swapChain, std::shared_ptr<Camera> camera, const std::string& environmentPath, std::vector<RenderObject>& renderObjects);
		~Scene();

		void draw(VkCommandBuffer commandBuffer, VkRenderPass renderPass);
		void updateBuffer(GPUCameraData data);
		void updateBuffer(GPUSceneData data);

	private:
		Device& device;

		std::shared_ptr<Camera> camera;
		std::unique_ptr<Cubemap> globalCubemap;
		std::vector<RenderObject> renderObjects;

		std::unique_ptr<Skybox> skybox;

		VkDescriptorSetLayout sceneSetLayout;
		AllocatedBuffer cameraBuffer;
		AllocatedBuffer sceneBuffer;
		std::vector<VkDescriptorSet> sceneDescriptorSets;

		VkDescriptorSetLayout objectSetLayout;
		std::vector<AllocatedBuffer> objectBuffers;
		std::vector<VkDescriptorSet> objectDescriptorSets;

		void bindScene(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
		void bindObjects(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
		void createDescriptorSetLayout();
		void createFramebuffers();

		void updateObjectBuffer();

		const int FRAMEBUFFER_COUNT = 1;
		int frameBufferIndex = 0;
		VkCommandBuffer currentBuffer;
		const int MAX_OBJECTS = 10000;
	};
}