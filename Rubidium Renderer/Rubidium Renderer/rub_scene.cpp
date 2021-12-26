#include "rub_scene.hpp"

#include "vk_util.hpp"

namespace rub
{
	Scene::Scene(Device& device, std::unique_ptr<SwapChain>& swapChain, std::shared_ptr<Camera> camera, const std::string& environmentPath, std::vector<RenderObject>& renderObjects)
		: device{ device }, renderObjects{ renderObjects }, FRAMEBUFFER_COUNT{ swapChain->MAX_FRAMES_IN_FLIGHT }, camera{ camera }
	{
		globalCubemap = std::make_unique<Cubemap>(device);
		skybox = std::make_unique<Skybox>(device, "textures/spruit_sunrise_2k.exr");

		createDescriptorSetLayout();
		createFramebuffers();
	}

	void Scene::createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding cameraBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);
		VkDescriptorSetLayoutBinding sceneBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		std::vector<VkDescriptorSetLayoutBinding> bindings = { cameraBinding, sceneBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.bindingCount = bindings.size();
		layoutInfo.flags = 0;
		layoutInfo.pBindings = bindings.data();

		vkCreateDescriptorSetLayout(device.getDevice(), &layoutInfo, nullptr, &sceneSetLayout);
	}

	void Scene::createFramebuffers()
	{
		size_t cameraSize = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUCameraData)) * FRAMEBUFFER_COUNT;
		device.createBuffer(cameraSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, cameraBuffer);

		size_t sceneSize = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUSceneData)) * FRAMEBUFFER_COUNT;
		device.createBuffer(sceneSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, sceneBuffer);

		sceneDescriptorSets.resize(FRAMEBUFFER_COUNT);
		for (int i = 0; i < FRAMEBUFFER_COUNT; i++)
		{
			device.getDescriptor(sceneSetLayout, sceneDescriptorSets[i]);

			VkDescriptorBufferInfo cameraBufferInfo{};
			cameraBufferInfo.buffer = cameraBuffer.buffer;
			cameraBufferInfo.range = sizeof(GPUCameraData);

			VkDescriptorBufferInfo sceneBufferInfo{};
			sceneBufferInfo.buffer = sceneBuffer.buffer;
			sceneBufferInfo.range = sizeof(GPUSceneData);

			VkWriteDescriptorSet cameraWrite = VkUtil::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, sceneDescriptorSets[i], &cameraBufferInfo, 0);
			VkWriteDescriptorSet sceneWrite = VkUtil::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, sceneDescriptorSets[i], &sceneBufferInfo, 1);
			std::vector<VkWriteDescriptorSet> setWrites = { cameraWrite, sceneWrite };

			vkUpdateDescriptorSets(device.getDevice(), setWrites.size(), setWrites.data(), 0, nullptr);
		}
	}

	void Scene::updateBuffer(GPUCameraData data)
	{
		char* cameraData;
		vmaMapMemory(device.getAllocator(), cameraBuffer.allocation, (void**)&cameraData);
		cameraData += VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUCameraData)) * frameBufferIndex;
		memcpy(cameraData, &data, sizeof(GPUCameraData));
		vmaUnmapMemory(device.getAllocator(), cameraBuffer.allocation);
	}

	void Scene::updateBuffer(GPUSceneData data)
	{
		char* sceneData;
		vmaMapMemory(device.getAllocator(), sceneBuffer.allocation, (void**)&sceneData);
		sceneData += VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUSceneData)) * frameBufferIndex;
		memcpy(sceneData, &data, sizeof(GPUSceneData));
		vmaUnmapMemory(device.getAllocator(), sceneBuffer.allocation);
	}

	void Scene::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
	{
		uint32_t cameraOffset = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUCameraData)) * frameBufferIndex;
		uint32_t sceneOffset = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUSceneData)) * frameBufferIndex;
		std::vector<uint32_t> offsets = { cameraOffset, sceneOffset };

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, 
			&sceneDescriptorSets[frameBufferIndex], sceneDescriptorSets.size(), offsets.data());

		//Increment frame count when new command buffer is passed in (meaning new frame has started)
		if (currentBuffer != commandBuffer)
		{
			currentBuffer = commandBuffer;
			frameBufferIndex = (frameBufferIndex + 1) % FRAMEBUFFER_COUNT;
		}
	}

	void Scene::draw(VkCommandBuffer commandBuffer, VkRenderPass renderPass)
	{
		GPUCameraData cameraData = {
			camera->getProjectionMatrix(),
			camera->getViewMatrix(),
			camera->getPosition()
		};
		updateBuffer(cameraData);

		std::shared_ptr<Material> skyboxMaterial = skybox->getMaterial();
		if (!skyboxMaterial->isReady())
		{
			std::vector<VkDescriptorSetLayout> setLayouts = { sceneSetLayout };
			skyboxMaterial->setup(setLayouts, renderPass);
		}
		bind(commandBuffer, skyboxMaterial->getLayout());
		skybox->draw(commandBuffer);
	}

	Scene::~Scene()
	{
		vkDestroyDescriptorSetLayout(device.getDevice(), sceneSetLayout, nullptr);
		vmaDestroyBuffer(device.getAllocator(), cameraBuffer.buffer, cameraBuffer.allocation);
		vmaDestroyBuffer(device.getAllocator(), sceneBuffer.buffer, sceneBuffer.allocation);
	}
}