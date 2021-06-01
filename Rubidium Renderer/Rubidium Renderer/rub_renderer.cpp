#include "rub_renderer.hpp"

#include <glm/glm.hpp>
#include <glm\ext\matrix_transform.hpp>
#include <glm\ext\matrix_clip_space.hpp>

#include <stdexcept>
#include <array>

namespace rub
{
	RubRenderer::RubRenderer(RubWindow& window, RubDevice& device) : rubWindow{ window }, rubDevice{ device }
	{
		recreateSwapChain();
		createCommandBuffers();
		createGlobalDescriptor();

		cameraData = std::make_unique<CameraData>(device, rubSwapChain, globalDescriptor.globalSetLayout);
	}

	void RubRenderer::createCommandBuffers()
	{
		commandBuffers.resize(rubSwapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = rubDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(rubDevice.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void RubRenderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(rubDevice.getDevice(), rubDevice.getCommandPool(), static_cast<float>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}

	void RubRenderer::recreateSwapChain()
	{
		auto extent = rubWindow.getExtent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = rubWindow.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(rubDevice.getDevice());
		rubSwapChain.reset();
		if (rubSwapChain == nullptr)
		{
			rubSwapChain = std::make_unique<RubSwapChain>(rubDevice, extent);
		}
		else
		{
			rubSwapChain = std::make_unique<RubSwapChain>(rubDevice, extent, std::move(rubSwapChain));
			if (rubSwapChain->imageCount() != commandBuffers.size())
			{
				freeCommandBuffers();
				createCommandBuffers();
			}
		}
	}

	VkCommandBuffer RubRenderer::beginFrame()
	{
		assert(!isFrameStarted && "can't call beginFrame while frame is in progress");

		auto result = rubSwapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		updateCamera();

		return commandBuffer;
	}

	void RubRenderer::updateCamera()
	{
		//camera position
		glm::vec3 camPos = { 0.0f, 0.0f, -3.0f };

		glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
		//camera projection
		glm::mat4 projection = glm::perspective(glm::radians(70.f), rubSwapChain->getExtentAspectRatio(), 0.1f, 200.0f);
		projection[1][1] *= -1;

		GPUCameraData gpuData{};
		gpuData.view = view;
		gpuData.projection = projection;

		cameraData->updateBuffers(gpuData);
	}

	void RubRenderer::endFrame()
	{
		assert(isFrameStarted && "can't call endFrame while frame isn't in progress");

		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = rubSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || rubWindow.wasWindowResized())
		{
			rubWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		isFrameStarted = false;
	}

	void RubRenderer::beginRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "can't call beginRenderPass if frame isn't in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "can't end render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = rubSwapChain->getRenderPass();
		renderPassInfo.framebuffer = rubSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = rubSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(rubSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(rubSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, rubSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void RubRenderer::endRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "can't call endRenderPass if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "can't end render pass on command buffer from a different frame");
		
		vkCmdEndRenderPass(commandBuffer);
	}

	void RubRenderer::createGlobalDescriptor()
	{
		VkDescriptorSetLayoutBinding camBufferBinding = {};
		camBufferBinding.binding = 0;
		camBufferBinding.descriptorCount = 1;
		camBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		camBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.bindingCount = 1;
		layoutInfo.flags = 0;
		layoutInfo.pBindings = &camBufferBinding;

		if (vkCreateDescriptorSetLayout(rubDevice.getDevice(), &layoutInfo, nullptr, &globalDescriptor.globalSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	RubRenderer::~RubRenderer()
	{
		freeCommandBuffers();
		vkDestroyDescriptorSetLayout(rubDevice.getDevice(), globalDescriptor.globalSetLayout, nullptr);
	}
}