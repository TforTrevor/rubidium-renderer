#pragma once

#include "rub_device.hpp"

namespace rub
{
	class Cubemap
	{
	public:
		Cubemap(Device& device);
		~Cubemap();

		void createImages();
		void createRenderPass();
		void createFramebuffer();
		void capture();

	private:
		Device& device;
		const VkExtent2D frameExtent = { 512, 512 };

		std::vector<AllocatedImage> captureImages;
		std::vector<VkImageView> captureImageViews;
		VkRenderPass renderPass;
		VkFramebuffer captureFramebuffer;
	};
}