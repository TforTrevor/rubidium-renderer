#include "rub_cubemap.hpp"

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
		VkImageCreateInfo captureImageInfo = VkUtil::imageCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
			captureExtent, 1);
		captureImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		captureImageInfo.arrayLayers = 6;
		vmaCreateImage(device.getAllocator(), &captureImageInfo, &imageAllocation, &captureImage.image, &captureImage.allocation, nullptr);

		VkImageViewCreateInfo captureViewInfo = VkUtil::imageViewCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, captureImage.image, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		captureViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		captureViewInfo.subresourceRange.layerCount = 6;
		vkCreateImageView(device.getDevice(), &captureViewInfo, nullptr, &captureImageView);

		//Create irradiance image
		VkImageCreateInfo irradianceImageInfo = VkUtil::imageCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
			irradianceExtent, 1);
		irradianceImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		irradianceImageInfo.arrayLayers = 6;
		vmaCreateImage(device.getAllocator(), &captureImageInfo, &imageAllocation, &irradianceImage.image, &irradianceImage.allocation, nullptr);

		VkImageViewCreateInfo irradianceViewInfo = VkUtil::imageViewCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, irradianceImage.image, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		irradianceViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		irradianceViewInfo.subresourceRange.layerCount = 6;
		vkCreateImageView(device.getDevice(), &irradianceViewInfo, nullptr, &irradianceImageView);

		//Create prefilter image
		VkImageCreateInfo prefilterImageInfo = VkUtil::imageCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
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
		capture(commandBuffer, renderObjects, captureFramebuffer, captureExtent, captureImage, captureImageView);
		convertImage(commandBuffer, captureImage, captureImageView, captureExtent, captureMipLevels, true);
		device.endSingleTimeCommands(commandBuffer);
		cleanup();
	}

	void Cubemap::captureIrradiance()
	{
		std::shared_ptr<Texture> captureTexture = std::make_shared<Texture>(device, captureImageView, captureMipLevels);
		std::shared_ptr<Material> captureMaterial = std::make_shared<Material>(device, "shaders/cubemap.vert.spv", "shaders/irradiance_convolution.frag.spv");
		captureMaterial->addTexture(captureTexture);

		RenderObject renderObject{};
		renderObject.model = cubeModel;
		renderObject.material = captureMaterial;
		renderObject.transform = { glm::vec3(0), glm::vec3(0) };
		std::vector<RenderObject> renderObjects = { renderObject };

		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
		capture(commandBuffer, renderObjects, irradianceFramebuffer, irradianceExtent, irradianceImage, irradianceImageView);
		convertImage(commandBuffer, irradianceImage, irradianceImageView, irradianceExtent, 1, false);
		device.endSingleTimeCommands(commandBuffer);
		cleanup();
	}

	void Cubemap::capturePrefilter()
	{
		std::shared_ptr<Texture> captureTexture = std::make_shared<Texture>(device, captureImageView, captureMipLevels);
		std::shared_ptr<Material> captureMaterial = std::make_shared<Material>(device, "shaders/cubemap.vert.spv", "shaders/prefilter.frag.spv");
		captureMaterial->addTexture(captureTexture);

		RenderObject renderObject{};
		renderObject.model = cubeModel;
		renderObject.material = captureMaterial;
		renderObject.transform = { glm::vec3(0), glm::vec3(0) };
		std::vector<RenderObject> renderObjects = { renderObject };

		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
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
		convertImage(commandBuffer, prefilterImage, prefilterImageView, captureExtent, captureMipLevels, false);
		device.endSingleTimeCommands(commandBuffer);

		for (AllocatedImage& image : destroyImages)
		{
			vmaDestroyImage(device.getAllocator(), image.image, image.allocation);
		}
		destroyImages.clear();
		destroyImageViews.clear();
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

	void Cubemap::convertImage(VkCommandBuffer commandBuffer, AllocatedImage& oldImage, VkImageView& oldImageView, VkExtent2D extent, int mipLevels, bool generateMips)
	{
		//Transfer old image layout from color attachment to transfer source
		VkImageSubresourceRange colorRange{};
		colorRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorRange.baseMipLevel = 0;
		colorRange.levelCount = generateMips ? 1 : mipLevels;
		colorRange.baseArrayLayer = 0;
		colorRange.layerCount = 6;

		VkImageMemoryBarrier colorToSource{};
		colorToSource.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		colorToSource.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorToSource.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		colorToSource.image = oldImage.image;
		colorToSource.subresourceRange = colorRange;
		colorToSource.srcAccessMask = 0;
		colorToSource.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &colorToSource);

		//Create new image with shader read layout
		AllocatedImage newImage;
		VkImageCreateInfo newImageInfo = VkUtil::imageCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, extent, mipLevels);
		newImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		newImageInfo.arrayLayers = 6;
		VmaAllocationCreateInfo allocationInfo{};
		allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		vmaCreateImage(device.getAllocator(), &newImageInfo, &allocationInfo, &newImage.image, &newImage.allocation, nullptr);

		//Transfer new image layout from undefined to transfer destination
		VkImageSubresourceRange undefinedRange{};
		undefinedRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		undefinedRange.baseMipLevel = 0;
		undefinedRange.levelCount = mipLevels;
		undefinedRange.baseArrayLayer = 0;
		undefinedRange.layerCount = 6;

		VkImageMemoryBarrier undefinedToDst{};
		undefinedToDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		undefinedToDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		undefinedToDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		undefinedToDst.image = newImage.image;
		undefinedToDst.subresourceRange = undefinedRange;
		undefinedToDst.srcAccessMask = 0;
		undefinedToDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &undefinedToDst);

		//Copy the old image into the new image
		int mipWidth = extent.width;
		int mipHeight = extent.height;
		for (int i = 0; i < mipLevels; i++)
		{
			int width = extent.width;
			int height = extent.height;
			int mipLevel = 0;
			if (!generateMips)
			{
				width = mipWidth;
				height = mipHeight;
				mipLevel = i;
			}
			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { width, height, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = mipLevel;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 6;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 6;

			vkCmdBlitImage(commandBuffer, oldImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}		

		//Destroy old image and image view
		destroyImages.push_back(oldImage);
		destroyImageViews.push_back(oldImageView);

		//Assign new image and create new image view
		oldImage = newImage;
		VkImageViewCreateInfo viewInfo = VkUtil::imageViewCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, oldImage.image, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.subresourceRange.layerCount = 6;
		vkCreateImageView(device.getDevice(), &viewInfo, nullptr, &oldImageView);

		//Transition image layout from transfer destination to shader read
		VkImageMemoryBarrier toReadable = undefinedToDst;
		toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toReadable);
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