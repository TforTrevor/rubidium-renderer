#include "rub_renderer.hpp"

#include "vk_util.hpp"

#include <glm/glm.hpp>
#include <glm\ext\matrix_transform.hpp>
#include <glm\ext\matrix_clip_space.hpp>

#include <stdexcept>
#include <array>

namespace rub
{
	Renderer::Renderer(Window& window, Device& device) : window{ window }, device{ device }
	{
		recreateSwapChain();
		createCommandBuffers();
		globalDescriptor = std::make_unique<GlobalDescriptor>(device, swapChain);
	}

	void Renderer::createCommandBuffers()
	{
		commandBuffers.resize(swapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = device.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(device.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void Renderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(device.getDevice(), device.getCommandPool(), static_cast<float>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}

	void Renderer::recreateSwapChain()
	{
		auto extent = window.getExtent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = window.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device.getDevice());
		swapChain.reset();
		if (swapChain == nullptr)
		{
			swapChain = std::make_unique<SwapChain>(device, extent);
		}
		else
		{
			swapChain = std::make_unique<SwapChain>(device, extent, std::move(swapChain));
			if (swapChain->imageCount() != commandBuffers.size())
			{
				freeCommandBuffers();
				createCommandBuffers();
			}
		}
	}

	VkCommandBuffer Renderer::beginFrame()
	{
		assert(!isFrameStarted && "can't call beginFrame while frame is in progress");

		auto result = swapChain->acquireNextImage(&currentImageIndex);

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

	void Renderer::updateCamera()
	{
		glm::mat4 view = camera->getMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(70.f), swapChain->getExtentAspectRatio(), 0.1f, 1000.0f);
		projection[1][1] *= -1;

		GlobalDescriptor::GPUCameraData cameraData{};
		cameraData.view = view;
		cameraData.projection = projection;
		cameraData.position = glm::vec4(camera->getPosition(), 0);

		GlobalDescriptor::GPUSceneData sceneData{};
		sceneData.ambientColor = glm::vec4(0.58f, 0.87f, 0.98f, 0);
		sceneData.sunDirection = glm::vec4(0, 0.5f, 0.5f, 0);
		sceneData.sunColor = glm::vec4(1.0f, 1.0f, 1.0f, 0);

		GlobalDescriptor::GPULightData lightData{};
		lightData.lightPositions[0] = glm::vec4(-1.5f, 0.0f, -3.0f, 0.0f);
		lightData.lightPositions[1] = glm::vec4(1.5f, 0.0f, -3.0f, 0.0f);
		lightData.lightPositions[2] = glm::vec4(0.0f, 2.0f, 2.0f, 0.0f);
		lightData.lightColors[0] = glm::vec4(1.0f, 1.0f, 1.0f, 5.0f);
		lightData.lightColors[1] = glm::vec4(1.0f, 1.0f, 1.0f, 5.0f);
		lightData.lightColors[2] = glm::vec4(0.5f, 0.5f, 1.0f, 25.0f);

		globalDescriptor->updateCameraBuffer(cameraData);
		globalDescriptor->updateSceneBuffer(sceneData);
		globalDescriptor->updateLightBuffer(lightData);
	}

	void Renderer::endFrame()
	{
		assert(isFrameStarted && "can't call endFrame while frame isn't in progress");

		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized())
		{
			window.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		isFrameStarted = false;
	}

	void Renderer::beginRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "can't call beginRenderPass if frame isn't in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "can't end render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = swapChain->getRenderPass();
		renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, swapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Renderer::endRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "can't call endRenderPass if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "can't end render pass on command buffer from a different frame");
		
		vkCmdEndRenderPass(commandBuffer);
	}

	Renderer::~Renderer()
	{
		freeCommandBuffers();
	}
}