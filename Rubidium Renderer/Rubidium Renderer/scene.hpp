#pragma once

#include "render_object.hpp"
#include "cubemap.hpp"
#include "camera.hpp"
#include "swap_chain.hpp"
#include "skybox.hpp"
#include "compute_shader.hpp"

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
			uint32_t prefilterMips;
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
		VkSampler irradianceSampler;
		VkSampler prefilterSampler;

		VkDescriptorSetLayout sceneSetLayout;
		AllocatedBuffer cameraBuffer;
		AllocatedBuffer sceneBuffer;
		std::vector<VkDescriptorSet> sceneDescriptorSets;

		VkDescriptorSetLayout objectSetLayout;
		std::vector<AllocatedBuffer> objectBuffers;
		std::vector<VkDescriptorSet> objectDescriptorSets;

		std::unique_ptr<ComputeShader> brdfShader;
		AllocatedImage brdfImage;
		VkImageView brdfImageView;
		VkSampler brdfSampler;

		void bindScene(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
		void bindObjects(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
		void createDescriptorSetLayout();
		void createFramebuffers();
		void createBRDF();

		void updateObjectBuffer();

		const int FRAMEBUFFER_COUNT = 1;
		int frameBufferIndex = 0;
		VkCommandBuffer currentBuffer;
		const int MAX_OBJECTS = 10000;
	};
}