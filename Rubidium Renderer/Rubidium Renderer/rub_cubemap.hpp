#pragma once

#include "rub_render_object.hpp"

namespace rub
{
	class Cubemap
	{
	public:
		struct GPUCameraData
		{
			glm::mat4 projection;
			glm::mat4 view[6];
		};

		Cubemap(Device& device);
		~Cubemap();

		void capture(std::vector<RenderObject>& renderObjects);
		VkImageView getCaptureImageView() { return captureImageView; }
		AllocatedImage getCaptureImage() { return captureImage; }

	private:
		Device& device;
		const VkExtent2D captureExtent = { 512, 512 };

		AllocatedImage captureImage;
		VkImageView captureImageView;
		VkRenderPass renderPass;
		VkFramebuffer captureFramebuffer;

		VkDescriptorSetLayout setLayout;
		AllocatedBuffer cameraBuffer;
		VkDescriptorSet descriptorSet;

		std::vector<AllocatedImage> destroyImages;
		std::vector<VkImageView> destroyImageViews;

		void createImages();
		void createRenderPass();
		void createFramebuffer();
		void createDescriptorSetLayout();
		void createDescriptorSet();
		void convertImage(VkCommandBuffer commandBuffer);
	};
}