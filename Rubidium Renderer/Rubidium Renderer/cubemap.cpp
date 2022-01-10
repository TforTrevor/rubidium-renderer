#include "cubemap.hpp"

#include <stdexcept>

namespace rub
{
	Cubemap::Cubemap(Device& device) : device{ device }
	{
		cubeModel = std::make_shared<Model>(device, "models/cube.obj");

		createImages();
		createRenderPass();
		createFramebuffer();
		createDescriptorSetLayout();
		createDescriptorSet();
	}

	void Cubemap::createImages()
	{
		VmaAllocationCreateInfo imageAllocation{};
		imageAllocation.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		//Create capture image
		VkImageCreateInfo captureImageInfo = VkUtil::imageCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
			captureExtent, 1);
		captureImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		captureImageInfo.arrayLayers = 6;
		vmaCreateImage(device.getAllocator(), &captureImageInfo, &imageAllocation, &captureImage.image, &captureImage.allocation, nullptr);

		VkImageViewCreateInfo captureViewInfo = VkUtil::imageViewCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, captureImage.image, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		captureViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		captureViewInfo.subresourceRange.layerCount = 6;
		vkCreateImageView(device.getDevice(), &captureViewInfo, nullptr, &captureImageView);

		//Create irradiance image
		VkImageCreateInfo irradianceImageInfo = VkUtil::imageCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			irradianceExtent, 1);
		irradianceImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		irradianceImageInfo.arrayLayers = 6;
		vmaCreateImage(device.getAllocator(), &captureImageInfo, &imageAllocation, &irradianceImage.image, &irradianceImage.allocation, nullptr);

		VkImageViewCreateInfo irradianceViewInfo = VkUtil::imageViewCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, irradianceImage.image, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		irradianceViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		irradianceViewInfo.subresourceRange.layerCount = 6;
		vkCreateImageView(device.getDevice(), &irradianceViewInfo, nullptr, &irradianceImageView);

		//Create prefilter image
		VkImageCreateInfo prefilterImageInfo = VkUtil::imageCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			captureExtent, captureMipLevels);
		prefilterImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		prefilterImageInfo.arrayLayers = 6;
		vmaCreateImage(device.getAllocator(), &prefilterImageInfo, &imageAllocation, &prefilterImage.image, &prefilterImage.allocation, nullptr);

		prefilterImageViews.resize(captureMipLevels);
		for (int i = 0; i < captureMipLevels; i++)
		{
			VkImageViewCreateInfo prefilterViewInfo = VkUtil::imageViewCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, prefilterImage.image, VK_IMAGE_ASPECT_COLOR_BIT, 1);
			prefilterViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			prefilterViewInfo.subresourceRange.layerCount = 6;
			prefilterViewInfo.subresourceRange.baseMipLevel = i;
			vkCreateImageView(device.getDevice(), &prefilterViewInfo, nullptr, &prefilterImageViews[i]);
		}		
	}

	void Cubemap::createRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstSubpass = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		unsigned int viewMask = 0b00111111;
		unsigned int correlationMask = 0b00000000;

		VkRenderPassMultiviewCreateInfo multiviewInfo{};
		multiviewInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
		multiviewInfo.subpassCount = 1;
		multiviewInfo.pViewMasks = &viewMask;
		multiviewInfo.correlationMaskCount = 1;
		multiviewInfo.pCorrelationMasks = &correlationMask;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pNext = &multiviewInfo;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device.getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void Cubemap::createFramebuffer()
	{
		VkFramebufferCreateInfo captureInfo{};
		captureInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		captureInfo.renderPass = renderPass;
		captureInfo.attachmentCount = 1;
		captureInfo.pAttachments = &captureImageView;
		captureInfo.width = captureExtent.width;
		captureInfo.height = captureExtent.height;
		captureInfo.layers = 1;
		vkCreateFramebuffer(device.getDevice(), &captureInfo, nullptr, &captureFramebuffer);

		VkFramebufferCreateInfo irradianceInfo{};
		irradianceInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		irradianceInfo.renderPass = renderPass;
		irradianceInfo.attachmentCount = 1;
		irradianceInfo.pAttachments = &irradianceImageView;
		irradianceInfo.width = irradianceExtent.width;
		irradianceInfo.height = irradianceExtent.height;
		irradianceInfo.layers = 1;
		vkCreateFramebuffer(device.getDevice(), &irradianceInfo, nullptr, &irradianceFramebuffer);

		prefilterFramebuffers.resize(captureMipLevels);
		for (int i = 0; i < prefilterFramebuffers.size(); i++)
		{
			int mipWidth = captureExtent.width * std::pow(0.5f, i);
			int mipHeight = captureExtent.height * std::pow(0.5f, i);

			VkFramebufferCreateInfo prefilterInfo{};
			prefilterInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			prefilterInfo.renderPass = renderPass;
			prefilterInfo.attachmentCount = 1;
			prefilterInfo.pAttachments = &prefilterImageViews[i];
			prefilterInfo.width = mipWidth;
			prefilterInfo.height = mipHeight;
			prefilterInfo.layers = 1;
			vkCreateFramebuffer(device.getDevice(), &prefilterInfo, nullptr, &prefilterFramebuffers[i]);
		}
	}

	void Cubemap::createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding cameraBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);
		VkDescriptorSetLayoutBinding prefilterBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		std::vector<VkDescriptorSetLayoutBinding> bindings = { cameraBinding, prefilterBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.bindingCount = bindings.size();
		layoutInfo.flags = 0;
		layoutInfo.pBindings = bindings.data();

		vkCreateDescriptorSetLayout(device.getDevice(), &layoutInfo, nullptr, &setLayout);
	}

	void Cubemap::createDescriptorSet()
	{
		size_t cameraSize = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUCameraData));
		device.createBuffer(cameraSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, cameraBuffer);

		size_t prefilterSize = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUPrefilterData)) * captureMipLevels;
		device.createBuffer(prefilterSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, prefilterBuffer);

		device.getDescriptor(setLayout, descriptorSet);

		VkDescriptorBufferInfo cameraBufferInfo{};
		cameraBufferInfo.buffer = cameraBuffer.buffer;
		cameraBufferInfo.range = sizeof(GPUCameraData);

		VkDescriptorBufferInfo prefilterBufferInfo{};
		prefilterBufferInfo.buffer = prefilterBuffer.buffer;
		prefilterBufferInfo.range = sizeof(GPUPrefilterData);

		VkWriteDescriptorSet cameraWrite = VkUtil::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorSet, &cameraBufferInfo, 0);
		VkWriteDescriptorSet prefilterWrite = VkUtil::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, descriptorSet, &prefilterBufferInfo, 1);
		std::vector<VkWriteDescriptorSet> setWrites = { cameraWrite, prefilterWrite };

		vkUpdateDescriptorSets(device.getDevice(), setWrites.size(), setWrites.data(), 0, nullptr);

		GPUCameraData gpuCameraData = {
			glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f),
			{
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
			}
		};
		void* cameraData;
		vmaMapMemory(device.getAllocator(), cameraBuffer.allocation, &cameraData);
		memcpy(cameraData, &gpuCameraData, sizeof(GPUCameraData));
		vmaUnmapMemory(device.getAllocator(), cameraBuffer.allocation);
	}

	void Cubemap::capture(std::vector<RenderObject>& renderObjects)
	{
		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
		
		captureEnvironment(commandBuffer, renderObjects);
		captureIrradiance(commandBuffer);
		capturePrefilter(commandBuffer);

		device.endSingleTimeCommands(commandBuffer);

		cleanup();
	}

	void Cubemap::captureEnvironment(VkCommandBuffer commandBuffer, std::vector<RenderObject>& renderObjects)
	{
		//Capture environment
		capture(commandBuffer, renderObjects, captureFramebuffer, captureExtent, captureImage, captureImageView);

		//Create new image with mips
		AllocatedImage newImage;
		VkImageCreateInfo captureImageInfo = VkUtil::imageCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			captureExtent, captureMipLevels);
		captureImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		captureImageInfo.arrayLayers = 6;
		VmaAllocationCreateInfo allocationInfo{};
		allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		vmaCreateImage(device.getAllocator(), &captureImageInfo, &allocationInfo, &newImage.image, &newImage.allocation, nullptr);

		//Generate mip maps
		VkUtil::generateMipMaps(commandBuffer, captureImage.image, newImage.image, captureExtent.width, captureExtent.height, 6);

		//Create new image view with mips
		VkImageView newImageView;
		VkImageViewCreateInfo captureViewInfo = VkUtil::imageViewCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, newImage.image, VK_IMAGE_ASPECT_COLOR_BIT, captureMipLevels);
		captureViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		captureViewInfo.subresourceRange.layerCount = 6;
		vkCreateImageView(device.getDevice(), &captureViewInfo, nullptr, &newImageView);

		destroyImages.push_back(captureImage);
		destroyImageViews.push_back(captureImageView);

		captureImage = newImage;
		captureImageView = newImageView;
	}

	void Cubemap::captureIrradiance(VkCommandBuffer commandBuffer)
	{
		//Create render object
		std::shared_ptr<Texture> irradianceTexture = std::make_shared<Texture>(device, captureImageView, captureMipLevels);
		irradianceMaterial = std::make_shared<Material>(device, "shaders/cubemap.vert.spv", "shaders/irradiance_convolution.frag.spv");
		irradianceMaterial->addTexture(irradianceTexture);

		RenderObject renderObject{};
		renderObject.model = cubeModel;
		renderObject.material = irradianceMaterial;
		renderObject.transform = { glm::vec3(0), glm::vec3(0) };
		std::vector<RenderObject> renderObjects = { renderObject };

		//Capture irradiance
		capture(commandBuffer, renderObjects, irradianceFramebuffer, irradianceExtent, irradianceImage, irradianceImageView);

		//Transition from color attachment to shader read
		AllocatedImage newImage;
		VkUtil::convertColorAttachmentToShaderRead(commandBuffer, device.getAllocator(), irradianceImage, newImage, irradianceExtent, 1, 6);

		//Create new image view
		VkImageView newImageView;
		VkImageViewCreateInfo viewInfo = VkUtil::imageViewCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, newImage.image, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.subresourceRange.layerCount = 6;
		vkCreateImageView(device.getDevice(), &viewInfo, nullptr, &newImageView);

		destroyImages.push_back(irradianceImage);
		destroyImageViews.push_back(irradianceImageView);

		irradianceImage = newImage;
		irradianceImageView = newImageView;
	}

	void Cubemap::capturePrefilter(VkCommandBuffer commandBuffer)
	{
		//Create render object
		std::shared_ptr<Texture> prefilterTexture = std::make_shared<Texture>(device, captureImageView, captureMipLevels);
		prefilterMaterial = std::make_shared<Material>(device, "shaders/cubemap.vert.spv", "shaders/prefilter.frag.spv");
		prefilterMaterial->addTexture(prefilterTexture);

		RenderObject renderObject{};
		renderObject.model = cubeModel;
		renderObject.material = prefilterMaterial;
		renderObject.transform = { glm::vec3(0), glm::vec3(0) };
		std::vector<RenderObject> renderObjects = { renderObject };

		//Capture prefilter
		for (int i = 0; i < captureMipLevels; i++)
		{
			int mipWidth = captureExtent.width * std::pow(0.5f, i);
			int mipHeight = captureExtent.height * std::pow(0.5f, i);
			VkExtent2D extent = { mipWidth, mipHeight };

			GPUPrefilterData gpuPrefilter;
			gpuPrefilter.roughness = (float)i / (float)(captureMipLevels - 1);

			char* prefilterData;
			vmaMapMemory(device.getAllocator(), prefilterBuffer.allocation, (void**)&prefilterData);
			prefilterData += VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUPrefilterData)) * i;
			memcpy(prefilterData, &gpuPrefilter, sizeof(GPUPrefilterData));
			vmaUnmapMemory(device.getAllocator(), prefilterBuffer.allocation);

			prefilterIndex = i;
			capture(commandBuffer, renderObjects, prefilterFramebuffers[i], extent, prefilterImage, prefilterImageViews[i]);
		}

		//Transition from color attachment to shader read
		AllocatedImage newImage;
		VkUtil::convertColorAttachmentToShaderRead(commandBuffer, device.getAllocator(), prefilterImage, newImage, captureExtent, captureMipLevels, 6);

		//Create new image view with mips
		VkImageView newImageView;
		VkImageViewCreateInfo viewInfo = VkUtil::imageViewCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, newImage.image, VK_IMAGE_ASPECT_COLOR_BIT, captureMipLevels);
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.subresourceRange.layerCount = 6;
		vkCreateImageView(device.getDevice(), &viewInfo, nullptr, &newImageView);

		destroyImages.push_back(prefilterImage);
		destroyImageViews.insert(destroyImageViews.end(), prefilterImageViews.begin(), prefilterImageViews.end());
		prefilterImageViews.clear();

		prefilterImage = newImage;
		prefilterImageView = newImageView;
	}

	void Cubemap::capture(VkCommandBuffer commandBuffer, std::vector<RenderObject>& renderObjects, VkFramebuffer frameBuffer, VkExtent2D extent, 
		AllocatedImage& image, VkImageView& imageView)
	{
		VkClearValue clearValue{};
		clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValue.depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = frameBuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = extent.width;
		viewport.height = extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, extent };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		std::vector<VkDescriptorSetLayout> setLayouts = { setLayout };
		uint32_t prefilterOffset = VkUtil::padUniformBufferSize(device.getDeviceProperties(), sizeof(GPUPrefilterData)) * prefilterIndex;
		std::vector<uint32_t> offsets = { prefilterOffset };

		for (int i = 0; i < renderObjects.size(); i++)
		{
			RenderObject& renderObject = renderObjects[i];

			if (!renderObject.material->isReady())
			{
				renderObject.material->setup(setLayouts, renderPass);
			}
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderObject.material->getLayout(), 0, 1, &descriptorSet, offsets.size(), offsets.data());
			renderObject.material->bind(commandBuffer);
			renderObject.model->bind(commandBuffer);
			renderObject.model->draw(commandBuffer, i);
		}

		vkCmdEndRenderPass(commandBuffer);
	}

	void Cubemap::cleanup()
	{
		//Destroy attachments
		for (AllocatedImage& image : destroyImages)
		{
			vmaDestroyImage(device.getAllocator(), image.image, image.allocation);
		}
		for (VkImageView& imageView : destroyImageViews)
		{
			vkDestroyImageView(device.getDevice(), imageView, nullptr);
		}
		destroyImages.clear();
		destroyImageViews.clear();
	}

	Cubemap::~Cubemap()
	{
		vmaDestroyImage(device.getAllocator(), captureImage.image, captureImage.allocation);
		vkDestroyImageView(device.getDevice(), captureImageView, nullptr);
		vkDestroyFramebuffer(device.getDevice(), captureFramebuffer, nullptr);

		vmaDestroyImage(device.getAllocator(), irradianceImage.image, irradianceImage.allocation);
		vkDestroyImageView(device.getDevice(), irradianceImageView, nullptr);
		vkDestroyFramebuffer(device.getDevice(), irradianceFramebuffer, nullptr);

		vmaDestroyImage(device.getAllocator(), prefilterImage.image, prefilterImage.allocation);
		vkDestroyImageView(device.getDevice(), prefilterImageView, nullptr);
		for (VkFramebuffer& frameBuffer : prefilterFramebuffers)
		{
			vkDestroyFramebuffer(device.getDevice(), frameBuffer, nullptr);
		}

		vkDestroyRenderPass(device.getDevice(), renderPass, nullptr);
		vkDestroyDescriptorSetLayout(device.getDevice(), setLayout, nullptr);
		vmaDestroyBuffer(device.getAllocator(), cameraBuffer.buffer, cameraBuffer.allocation);
		vmaDestroyBuffer(device.getAllocator(), prefilterBuffer.buffer, prefilterBuffer.allocation);

		for (VkImageView& imageView : prefilterImageViews)
		{
			vkDestroyImageView(device.getDevice(), imageView, nullptr);
		}
	}
}