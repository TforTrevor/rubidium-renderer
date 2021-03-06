#pragma once

#include "device.hpp"

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
		Texture(Device& device, const std::string& file, Format format);
		Texture(Device& device, VkImageView imageView, int mipLevels);
		~Texture();

		AllocatedImage getImage() { return allocatedImage; }
		VkImageView getImageView() { return imageView; }
		int getMipLevels() { return mipLevels; }
	private:
		Device& device;
		AllocatedImage allocatedImage;
		VkImageView imageView;

		const int mipLevels = 1;
		bool ownsImage = true;

		bool createHDRImage(const std::string& file);
		bool createSDRImage(const std::string& file, Format format);
		void transferToGPU(const int width, const int height, Format format, AllocatedBuffer& stagingBuffer);
		void transitionImageLayout(AllocatedBuffer staging, AllocatedImage newImage, Format format, VkExtent2D imageExtent);
		void createImageView(Format format);
	};
}