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
	}

	void Cubemap::createImages()
	{
		const int imageCount = 1;
		captureImages.resize(imageCount);
		captureImageViews.resize(imageCount);
		for (int i = 0; i < imageCount; i++)
		{
			VkImageCreateInfo imageInfo = VkUtil::imageCreateInfo(VK_FORMAT_R16G16B16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, frameExtent);
			VmaAllocationCreateInfo allocationInfo = {};
			allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			vmaCreateImage(device.getAllocator(), &imageInfo, &allocationInfo, &captureImages[i].image, &captureImages[i].allocation, nullptr);

			VkImageViewCreateInfo viewInfo = VkUtil::imageViewCreateInfo(VK_FORMAT_R16G16B16_SFLOAT, captureImages[i].image, VK_IMAGE_ASPECT_COLOR_BIT);
			if (vkCreateImageView(device.getDevice(), &viewInfo, nullptr, &captureImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create texture image view!");
			}
		}
	}

	void Cubemap::createRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_R16G16B16_SFLOAT;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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
		framebufferInfo.width = frameExtent.width;
		framebufferInfo.height = frameExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device.getDevice(), &framebufferInfo, nullptr, &captureFramebuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}

	void Cubemap::capture()
	{
		VkClearValue clearValue{};
		clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValue.depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = captureFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = frameExtent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearValue;

		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
		{
			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = frameExtent.width;
			viewport.height = frameExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			VkRect2D scissor{ {0, 0}, frameExtent };
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			vkCmdEndRenderPass(commandBuffer);
		}
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
	}
}