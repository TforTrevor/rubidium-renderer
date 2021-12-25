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

	private:
		Device& device;
		const VkExtent2D captureExtent = { 512, 512 };

		std::vector<AllocatedImage> captureImages;
		std::vector<VkImageView> captureImageViews;
		VkRenderPass renderPass;
		VkFramebuffer captureFramebuffer;

		VkDescriptorSetLayout setLayout;
		AllocatedBuffer cameraBuffer;
		VkDescriptorSet descriptorSet;

		void createImages();
		void createRenderPass();
		void createFramebuffer();
		void createDescriptorSetLayout();
		void createDescriptorSet();
	};
}