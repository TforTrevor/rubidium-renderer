#pragma once

#include "rub_device.hpp"

namespace rub
{
	class Texture
	{
	public:
		enum Format
		{
			SRGB = VK_FORMAT_R8G8B8A8_SRGB,
			LINEAR = VK_FORMAT_R8G8B8A8_UNORM,
			HDR = VK_FORMAT_R32G32B32A32_SFLOAT
		};
		Texture(Device& device, const char* file, Format format);
		~Texture();

		AllocatedImage getImage() { return allocatedImage; }
		VkImageView getImageView() { return imageView; }
	private:
		bool createImage(const char* file, Format format);
		void transitionImageLayout(AllocatedBuffer staging, AllocatedImage newImage, Format format, VkExtent2D imageExtent);
		void createImageView(Format format);

		Device& device;
		AllocatedImage allocatedImage;
		VkImageView imageView;
	};
}