#pragma once

#include "rub_device.hpp"

namespace rub
{
	class Texture
	{
	public:
		Texture(Device& device, const char* file, VkFormat format);
		~Texture();

		AllocatedImage getImage() { return allocatedImage; }
		VkImageView getImageView() { return imageView; }
	private:
		bool createImage(const char* file, VkFormat format);
		void transitionImageLayout(AllocatedBuffer staging, AllocatedImage newImage, VkFormat format, VkExtent3D imageExtent);
		void createImageView(VkFormat format);

		Device& device;
		AllocatedImage allocatedImage;
		VkImageView imageView;
	};
}