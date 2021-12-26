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
		Scene(Device& device, std::unique_ptr<SwapChain>& swapChain, std::shared_ptr<Camera> camera, const std::string& environmentPath, std::vector<RenderObject>& renderObjects);
		~Scene();

		struct GPUCameraData
		{
			glm::mat4 view;
			glm::mat4 projection;
			glm::vec3 position;
		};

		struct GPUSceneData
		{
			glm::vec4 ambientColor;
		};

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

		void bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
		void createDescriptorSetLayout();
		void createFramebuffers();

		const int FRAMEBUFFER_COUNT = 1;
		int frameBufferIndex = 0;
		VkCommandBuffer currentBuffer;
	};
}