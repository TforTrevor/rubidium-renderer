#include "rub_scene.hpp"

#include "vk_util.hpp"

namespace rub
{
	Scene::Scene(Device& device, std::unique_ptr<SwapChain>& swapChain, std::shared_ptr<Camera> camera, const std::string& environmentPath, std::vector<RenderObject>& renderObjects)
		: device{ device }, renderObjects{ renderObjects }, FRAMEBUFFER_COUNT{ swapChain->MAX_FRAMES_IN_FLIGHT }, camera{ camera }
	{
		globalCubemap = std::make_unique<Cubemap>(device);
		skybox = std::make_unique<Skybox>(device, "textures/spruit_sunrise_2k.exr");

		createBRDF();
		createDescriptorSetLayout();
		createFramebuffers();
	}

	void Scene::createBRDF()
	{
		VkExtent2D extent = { 512, 512 };

		VkImageCreateInfo imageInfo = VkUtil::imageCreateInfo(VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, extent, 1);
		VmaAllocationCreateInfo allocationInfo{};
		allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		vmaCreateImage(device.getAllocator(), &imageInfo, &allocationInfo, &brdfImage.image, &brdfImage.allocation, nullptr);

		VkImageViewCreateInfo viewInfo = VkUtil::imageViewCreateInfo(VK_FORMAT_R16G16_SFLOAT, brdfImage.image, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		vkCreateImageView(device.getDevice(), &viewInfo, nullptr, &brdfImageView);		

		std::vector<VkDescriptorSetLayout> empty;
		brdfShader = std::make_unique<ComputeShader>(device, "shaders/brdf.comp.spv", brdfImageView, empty);
		
		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
		VkUtil::transitionUndefinedToGeneral(commandBuffer, brdfImage.image, 1);
		brdfShader->bind(commandBuffer);
		vkCmdDispatch(commandBuffer, 32, 32, 1);
		VkUtil::transitionGeneralToShader(commandBuffer, brdfImage.image, 1);
		device.endSingleTimeCommands(commandBuffer);
	}

	void Scene::createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding cameraBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);
		VkDescriptorSetLayoutBinding sceneBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		VkDescriptorSetLayoutBinding irradianceBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
		VkDescriptorSetLayoutBinding prefilterBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);
		VkDescriptorSetLayoutBinding brdfBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4);
		std::vector<VkDescriptorSetLayoutBinding> sceneBindings = { cameraBinding, sceneBinding, irradianceBinding, prefilterBinding, brdfBinding };

		VkDescriptorSetLayoutCreateInfo sceneLayoutInfo{};
		sceneLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		sceneLayoutInfo.pNext = nullptr;
		sceneLayoutInfo.flags = 0;
		sceneLayoutInfo.bindingCount = sceneBindings.size();
		sceneLayoutInfo.pBindings = sceneBindings.data();

		vkCreateDescriptorSetLayout(device.getDevice(), &sceneLayoutInfo, nullptr, &sceneSetLayout);

		VkDescriptorSetLayoutBinding objectBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
		std::vector<VkDescriptorSetLayoutBinding> objectBindings = { objectBinding };

		VkDescriptorSetLayoutCreateInfo objectLayoutInfo{};
		objectLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		objectLayoutInfo.pNext = nullptr;
		objectLayoutInfo.flags = 0;
		objectLayoutInfo.bindingCount = objectBindings.size();
		objectLayoutInfo.pBindings = objectBindings.data();

		vkCreateDescriptorSetLayout(device.getDevice(), &objectLayoutInfo, nullptr, &objectSetLayout);
	}

	void Scene::createFramebuffers()
	{
		size_t cameraSize = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUCameraData)) * FRAMEBUFFER_COUNT;
		device.createBuffer(cameraSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, cameraBuffer);

		size_t sceneSize = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUSceneData)) * FRAMEBUFFER_COUNT;
		device.createBuffer(sceneSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, sceneBuffer);

		VkSamplerCreateInfo irradianceSamplerInfo = VkUtil::samplesCreateInfo(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 1);
		vkCreateSampler(device.getDevice(), &irradianceSamplerInfo, nullptr, &irradianceSampler);
		VkSamplerCreateInfo prefilterSamplerInfo = VkUtil::samplesCreateInfo(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, skybox->getPrefilterMipLevels());
		vkCreateSampler(device.getDevice(), &prefilterSamplerInfo, nullptr, &prefilterSampler);
		VkSamplerCreateInfo brdfSamplerInfo = VkUtil::samplesCreateInfo(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 1);
		vkCreateSampler(device.getDevice(), &brdfSamplerInfo, nullptr, &brdfSampler);

		VkDescriptorImageInfo irradianceInfo{};
		irradianceInfo.sampler = irradianceSampler;
		irradianceInfo.imageView = skybox->getIrradiance();
		irradianceInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo prefilterInfo{};
		prefilterInfo.sampler = prefilterSampler;
		prefilterInfo.imageView = skybox->getPrefilter();
		prefilterInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo brdfInfo{};
		brdfInfo.sampler = brdfSampler;
		brdfInfo.imageView = brdfImageView;
		brdfInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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
			VkWriteDescriptorSet irradianceWrite = VkUtil::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, sceneDescriptorSets[i], &irradianceInfo, 2);
			VkWriteDescriptorSet prefilterWrite = VkUtil::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, sceneDescriptorSets[i], &prefilterInfo, 3);
			VkWriteDescriptorSet brdfWrite = VkUtil::writeDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, sceneDescriptorSets[i], &brdfInfo, 4);
			std::vector<VkWriteDescriptorSet> setWrites = { cameraWrite, sceneWrite, irradianceWrite, prefilterWrite, brdfWrite };

			vkUpdateDescriptorSets(device.getDevice(), setWrites.size(), setWrites.data(), 0, nullptr);
		}

		objectDescriptorSets.resize(FRAMEBUFFER_COUNT);
		objectBuffers.resize(FRAMEBUFFER_COUNT);
		for (int i = 0; i < FRAMEBUFFER_COUNT; i++)
		{
			size_t bufferSize = sizeof(GPUObjectData) * MAX_OBJECTS;
			device.createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, objectBuffers[i]);
			device.getDescriptor(objectSetLayout, objectDescriptorSets[i]);

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = objectBuffers[i].buffer;
			bufferInfo.range = bufferSize;

			VkWriteDescriptorSet setWrite = VkUtil::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, objectDescriptorSets[i], &bufferInfo, 0);
			vkUpdateDescriptorSets(device.getDevice(), 1, &setWrite, 0, nullptr);
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

	void Scene::updateObjectBuffer()
	{
		void* objectData;
		vmaMapMemory(device.getAllocator(), objectBuffers[frameBufferIndex].allocation, &objectData);
		GPUObjectData* objectSSBO = (GPUObjectData*)objectData;

		for (int i = 0; i < renderObjects.size(); i++)
		{
			auto& object = renderObjects[i];
			objectSSBO[i].modelMatrix = object.transform.getMatrix();
			objectSSBO[i].MVP = camera->getProjectionMatrix() * camera->getViewMatrix() * object.transform.getMatrix();
		}

		vmaUnmapMemory(device.getAllocator(), objectBuffers[frameBufferIndex].allocation);
	}

	void Scene::bindScene(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
	{
		uint32_t cameraOffset = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUCameraData)) * frameBufferIndex;
		uint32_t sceneOffset = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUSceneData)) * frameBufferIndex;
		std::vector<uint32_t> offsets = { cameraOffset, sceneOffset };

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, 
			&sceneDescriptorSets[frameBufferIndex], sceneDescriptorSets.size(), offsets.data());
	}

	void Scene::bindObjects(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1,
			&objectDescriptorSets[frameBufferIndex], 0, nullptr);
	}

	void Scene::draw(VkCommandBuffer commandBuffer, VkRenderPass renderPass)
	{
		for (RenderObject& object : renderObjects)
		{
			object.transform.rotate(glm::vec3(0, 0.4f, 0));
		}

		//Update buffers
		GPUCameraData cameraData{};
		cameraData.projection = camera->getProjectionMatrix();
		cameraData.view = camera->getViewMatrix();
		cameraData.position = glm::vec4(camera->getPosition(), 1.0f);
		updateBuffer(cameraData);

		GPUSceneData sceneData{};
		sceneData.ambientColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
		sceneData.sunDirection = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
		sceneData.sunColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		sceneData.lightPositions[0] = glm::vec4(-1.5f, 0.0f, -3.0f, 0.0f);
		sceneData.lightPositions[1] = glm::vec4(1.5f, 0.0f, -3.0f, 0.0f);
		sceneData.lightPositions[2] = glm::vec4(0.0f, 2.0f, 2.0f, 0.0f);
		sceneData.lightColors[0] = glm::vec4(1.0f, 1.0f, 1.0f, 5.0f);
		sceneData.lightColors[1] = glm::vec4(1.0f, 1.0f, 1.0f, 5.0f);
		sceneData.lightColors[2] = glm::vec4(0.5f, 0.5f, 1.0f, 25.0f);
		sceneData.lightCount = 0;
		sceneData.prefilterMips = skybox->getPrefilterMipLevels();
		updateBuffer(sceneData);

		updateObjectBuffer();

		//Draw render objects
		for (int i = 0; i < renderObjects.size(); i++)
		{
			RenderObject& object = renderObjects[i];

			std::shared_ptr<Material> material = object.material;
			std::shared_ptr<Model> model = object.model;

			if (!material->isReady())
			{
				std::vector<VkDescriptorSetLayout> setLayouts = { sceneSetLayout, objectSetLayout };
				material->setup(setLayouts, renderPass);
			}
			bindScene(commandBuffer, material->getLayout());
			bindObjects(commandBuffer, material->getLayout());
			material->bind(commandBuffer);
			model->bind(commandBuffer);
			model->draw(commandBuffer, i);
		}

		//Draw skybox last
		std::shared_ptr<Material> skyboxMaterial = skybox->getMaterial();
		if (!skyboxMaterial->isReady())
		{
			std::vector<VkDescriptorSetLayout> setLayouts = { sceneSetLayout };
			skyboxMaterial->setup(setLayouts, renderPass);
		}
		bindScene(commandBuffer, skyboxMaterial->getLayout());
		skybox->draw(commandBuffer);

		//Increment frame count when new command buffer is passed in (meaning new frame has started)
		if (currentBuffer != commandBuffer)
		{
			currentBuffer = commandBuffer;
			frameBufferIndex = (frameBufferIndex + 1) % FRAMEBUFFER_COUNT;
		}
	}

	Scene::~Scene()
	{
		vkDestroyDescriptorSetLayout(device.getDevice(), sceneSetLayout, nullptr);
		vmaDestroyBuffer(device.getAllocator(), cameraBuffer.buffer, cameraBuffer.allocation);
		vmaDestroyBuffer(device.getAllocator(), sceneBuffer.buffer, sceneBuffer.allocation);

		vkDestroyDescriptorSetLayout(device.getDevice(), objectSetLayout, nullptr);
		for (AllocatedBuffer& buffer : objectBuffers)
		{
			vmaDestroyBuffer(device.getAllocator(), buffer.buffer, buffer.allocation);
		}

		vkDestroySampler(device.getDevice(), irradianceSampler, nullptr);
		vkDestroySampler(device.getDevice(), prefilterSampler, nullptr);

		vmaDestroyImage(device.getAllocator(), brdfImage.image, brdfImage.allocation);
		vkDestroyImageView(device.getDevice(), brdfImageView, nullptr);
		vkDestroySampler(device.getDevice(), brdfSampler, nullptr);
	}
}