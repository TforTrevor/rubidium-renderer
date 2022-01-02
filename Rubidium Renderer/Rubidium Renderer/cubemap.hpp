#pragma once

#include "render_object.hpp"
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
		struct GPUPrefilterData
		{
			float roughness;
		};

		Cubemap(Device& device);
		~Cubemap();

		void capture(std::vector<RenderObject>& renderObjects);
		VkImageView getCaptureImageView() { return captureImageView; }
		AllocatedImage getCaptureImage() { return captureImage; }
		VkImageView getIrradianceImageView() { return irradianceImageView; }
		AllocatedImage getIrradianceImage() { return irradianceImage; }
		VkImageView getPrefilterImageView() { return prefilterImageView; }
		AllocatedImage getPrefilterImage() { return prefilterImage; }
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
		AllocatedImage prefilterImage;
		VkImageView prefilterImageView;
		std::vector<VkImageView> prefilterImageViews;
		std::vector<VkFramebuffer> prefilterFramebuffers;

		VkDescriptorSetLayout setLayout;
		AllocatedBuffer cameraBuffer;
		AllocatedBuffer prefilterBuffer;
		VkDescriptorSet descriptorSet;

		std::vector<AllocatedImage> destroyImages;
		std::vector<VkImageView> destroyImageViews;

		std::shared_ptr<Model> cubeModel;
		std::shared_ptr<Material> irradianceMaterial;
		std::shared_ptr<Material> prefilterMaterial;

		int prefilterIndex;

		void createImages();
		void createRenderPass();
		void createFramebuffer();
		void createDescriptorSetLayout();
		void createDescriptorSet();
		void cleanup();

		void captureEnvironment(VkCommandBuffer commandBuffer, std::vector<RenderObject>& renderObjects);
		void captureIrradiance(VkCommandBuffer commandBuffer);
		void capturePrefilter(VkCommandBuffer commandBuffer);
		void capture(VkCommandBuffer commandBuffer, std::vector<RenderObject>& renderObjects, VkFramebuffer frameBuffer, VkExtent2D extent, 
			AllocatedImage& image, VkImageView& imageView);
		//void convertImage(VkCommandBuffer commandBuffer, AllocatedImage& oldImage, VkImageView& oldImageView, VkExtent2D extent, int mipLevels, bool generateMips);
	};
}