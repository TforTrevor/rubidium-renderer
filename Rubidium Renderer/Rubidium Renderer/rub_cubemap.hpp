#pragma once

#include "rub_render_object.hpp"
#include "vk_util.hpp"

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
		void captureIrradiance();
		VkImageView getCaptureImageView() { return captureImageView; }
		AllocatedImage getCaptureImage() { return captureImage; }
		VkImageView getIrradianceImageView() { return irradianceImageView; }
		AllocatedImage getIrradianceImage() { return irradianceImage; }
		int getCaptureMipLevels() { return captureMipLevels; }

	private:
		Device& device;
		const VkExtent2D captureExtent = { 512, 512 };
		const VkExtent2D irradianceExtent = { 32, 32 };
		const int captureMipLevels = VkUtil::calculateMipLevels(captureExtent.width, captureExtent.height);;

		VkRenderPass renderPass;

		AllocatedImage captureImage;
		VkImageView captureImageView;
		VkFramebuffer captureFramebuffer;
		AllocatedImage irradianceImage;
		VkImageView irradianceImageView;	
		VkFramebuffer irradianceFramebuffer;

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

		void capture(std::vector<RenderObject>& renderObjects, VkFramebuffer frameBuffer, VkExtent2D extent, AllocatedImage& image, VkImageView& imageView, int mipLevels);
		void convertImage(VkCommandBuffer commandBuffer, AllocatedImage& oldImage, VkImageView& oldImageView, VkExtent2D extent, int mipLevels);
	};
}