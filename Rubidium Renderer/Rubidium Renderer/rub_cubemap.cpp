#include "rub_cubemap.hpp"

#include "vk_util.hpp"
#include <stdexcept>

namespace rub
{
	Cubemap::Cubemap(Device& device) : device{ device }
	{
		createImages();
		createRenderPass();
		createFramebuffer();
		createDescriptorSetLayout();
		createDescriptorSet();
	}

	void Cubemap::createImages()
	{
		const int imageCount = 6;
		captureImages.resize(imageCount);
		captureImageViews.resize(imageCount);
		for (int i = 0; i < imageCount; i++)
		{
			VkImageCreateInfo imageInfo = VkUtil::imageCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, captureExtent);
			//imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			//imageInfo.arrayLayers = 6;

			VmaAllocationCreateInfo allocationInfo = {};
			allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			vmaCreateImage(device.getAllocator(), &imageInfo, &allocationInfo, &captureImages[i].image, &captureImages[i].allocation, nullptr);

			VkImageViewCreateInfo viewInfo = VkUtil::imageViewCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, captureImages[i].image, VK_IMAGE_ASPECT_COLOR_BIT);
			//viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			//viewInfo.subresourceRange.layerCount = 6;

			if (vkCreateImageView(device.getDevice(), &viewInfo, nullptr, &captureImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create texture image view!");
			}
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

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
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
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &captureImageViews[0];
		framebufferInfo.width = captureExtent.width;
		framebufferInfo.height = captureExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device.getDevice(), &framebufferInfo, nullptr, &captureFramebuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}

	void Cubemap::createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding cameraBinding = VkUtil::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);
		std::vector<VkDescriptorSetLayoutBinding> bindings = { cameraBinding };

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

		device.getDescriptor(setLayout, descriptorSet);

		VkDescriptorBufferInfo cameraBufferInfo{};
		cameraBufferInfo.buffer = cameraBuffer.buffer;
		cameraBufferInfo.range = sizeof(GPUCameraData);

		VkWriteDescriptorSet cameraWrite = VkUtil::writeDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorSet, &cameraBufferInfo, 0);
		std::vector<VkWriteDescriptorSet> setWrites = { cameraWrite };

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

		char* cameraData;
		vmaMapMemory(device.getAllocator(), cameraBuffer.allocation, (void**)&cameraData);
		memcpy(cameraData, &gpuCameraData, sizeof(GPUCameraData));
		vmaUnmapMemory(device.getAllocator(), cameraBuffer.allocation);
	}

	void Cubemap::capture(std::vector<RenderObject>& renderObjects)
	{
		VkClearValue clearValue{};
		clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValue.depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = captureFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = captureExtent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearValue;

		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = captureExtent.width;
		viewport.height = captureExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, captureExtent };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		std::vector<VkDescriptorSetLayout> setLayouts = { setLayout };

		for (int i = 0; i < renderObjects.size(); i++)
		{
			RenderObject& renderObject = renderObjects[i];

			if (!renderObject.material->isReady())
			{
				renderObject.material->setup(setLayouts, renderPass);
			}
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderObject.material->getLayout(), 0, 1, &descriptorSet, 0, nullptr);
			renderObject.material->bind(commandBuffer);
			renderObject.model->bind(commandBuffer);
			renderObject.model->draw(commandBuffer, i);
		}

		vkCmdEndRenderPass(commandBuffer);
		device.endSingleTimeCommands(commandBuffer);
	}

	Cubemap::~Cubemap()
	{
		for (AllocatedImage& image : captureImages)
		{
			vmaDestroyImage(device.getAllocator(), image.image, image.allocation);
		}

		for (VkImageView& imageView : captureImageViews)
		{
			vkDestroyImageView(device.getDevice(), imageView, nullptr);
		}

		vkDestroyFramebuffer(device.getDevice(), captureFramebuffer, nullptr);
		vkDestroyRenderPass(device.getDevice(), renderPass, nullptr);
		vkDestroyDescriptorSetLayout(device.getDevice(), setLayout, nullptr);
		vmaDestroyBuffer(device.getAllocator(), cameraBuffer.buffer, cameraBuffer.allocation);
	}
}