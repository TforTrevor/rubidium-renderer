#include "rub_texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h""

#include <iostream>

#include "vk_util.hpp"

namespace rub
{
	Texture::Texture(Device& device, const char* file, VkFormat format) : device{ device }
	{
		if (createImage(file, format))
		{
			createImageView(format);
		}		
	}

	bool Texture::createImage(const char* file, VkFormat format)
	{
		int texWidth, texHeight, texChannels;

		stbi_uc* pixels = stbi_load(file, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			std::cout << "Failed to load texture file " << file << std::endl;
			return true;
		}

		void* pixel_ptr = pixels;
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		//allocate temporary buffer for holding texture data to upload
		AllocatedBuffer stagingBuffer;
		device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

		//copy data to buffer
		void* data;
		vmaMapMemory(device.getAllocator(), stagingBuffer.allocation, &data);
		memcpy(data, pixel_ptr, static_cast<size_t>(imageSize));
		vmaUnmapMemory(device.getAllocator(), stagingBuffer.allocation);
		//we no longer need the loaded data, so we can free the pixels as they are now in the staging buffer
		stbi_image_free(pixels);



		VkExtent3D imageExtent;
		imageExtent.width = static_cast<uint32_t>(texWidth);
		imageExtent.height = static_cast<uint32_t>(texHeight);
		imageExtent.depth = 1;

		VkImageCreateInfo createInfo = VkUtil::imageCreateInfo(format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

		AllocatedImage newImage;

		VmaAllocationCreateInfo allocationInfo = {};
		allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		//allocate and create the image
		vmaCreateImage(device.getAllocator(), &createInfo, &allocationInfo, &newImage.image, &newImage.allocation, nullptr);

		transitionImageLayout(stagingBuffer, newImage, format, imageExtent);

		vmaDestroyBuffer(device.getAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);

		allocatedImage = newImage;

		return true;
	}

	void Texture::transitionImageLayout(AllocatedBuffer staging, AllocatedImage newImage, VkFormat format, VkExtent3D imageExtent)
	{
		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		VkImageMemoryBarrier toTransfer = {};
		toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		toTransfer.image = newImage.image;
		toTransfer.subresourceRange = range;
		toTransfer.srcAccessMask = 0;
		toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		//barrier the image into the transfer-receive layout
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toTransfer);

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;
		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = imageExtent;

		//copy the buffer into the image
		vkCmdCopyBufferToImage(commandBuffer, staging.buffer, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		VkImageMemoryBarrier toReadable = toTransfer;
		toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		//barrier the image into the shader readable layout
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toReadable);

		device.endSingleTimeCommands(commandBuffer);
	}

	void Texture::createImageView(VkFormat format)
	{
		VkImageViewCreateInfo imageinfo = VkUtil::imageViewCreateInfo(format, allocatedImage.image, VK_IMAGE_ASPECT_COLOR_BIT);
		vkCreateImageView(device.getDevice(), &imageinfo, nullptr, &imageView);
	}

	Texture::~Texture()
	{
		vmaDestroyImage(device.getAllocator(), allocatedImage.image, allocatedImage.allocation);
		vkDestroyImageView(device.getDevice(), imageView, nullptr);
	}
}