#pragma once

#include <cmath>

namespace rub
{
	class VkUtil
	{
	public:
		static VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding)
		{
			VkDescriptorSetLayoutBinding setbind = {};
			setbind.binding = binding;
			setbind.descriptorCount = 1;
			setbind.descriptorType = type;
			setbind.pImmutableSamplers = nullptr;
			setbind.stageFlags = stageFlags;

			return setbind;
		}

		static VkWriteDescriptorSet writeDescriptorBuffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding)
		{
			VkWriteDescriptorSet write = {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;

			write.dstBinding = binding;
			write.dstSet = dstSet;
			write.descriptorCount = 1;
			write.descriptorType = type;
			write.pBufferInfo = bufferInfo;

			return write;
		}

		static size_t padUniformBufferSize(VkPhysicalDeviceProperties deviceProperties, size_t originalSize)
		{
			// Calculate required alignment based on minimum device offset alignment
			size_t minUboAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
			size_t alignedSize = originalSize;
			if (minUboAlignment > 0)
			{
				alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
			}
			return alignedSize;
		}

		static void copyBuffer(Device& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
		{
			VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

			VkBufferCopy copyRegion{};
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

			device.endSingleTimeCommands(commandBuffer);
		}

		static VkImageCreateInfo imageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent2D extent, int mipLevels)
		{
			VkImageCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			info.pNext = nullptr;
			info.imageType = VK_IMAGE_TYPE_2D;
			info.format = format;
			info.extent = { extent.width, extent.height, 1 };
			info.mipLevels = mipLevels;
			info.arrayLayers = 1;
			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.tiling = VK_IMAGE_TILING_OPTIMAL;
			info.usage = usageFlags;

			return info;
		}

		static VkImageViewCreateInfo imageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags, int mipLevels)
		{
			VkImageViewCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.pNext = nullptr;
			info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.image = image;
			info.format = format;
			info.subresourceRange.baseMipLevel = 0;
			info.subresourceRange.levelCount = mipLevels;
			info.subresourceRange.baseArrayLayer = 0;
			info.subresourceRange.layerCount = 1;
			info.subresourceRange.aspectMask = aspectFlags;

			return info;
		}

		static VkSamplerCreateInfo samplesCreateInfo(VkFilter filters, VkSamplerAddressMode samplerAddressMode /*= VK_SAMPLER_ADDRESS_MODE_REPEAT*/, int mipLevels)
		{
			VkSamplerCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.pNext = nullptr;
			info.magFilter = filters;
			info.minFilter = filters;
			info.addressModeU = samplerAddressMode;
			info.addressModeV = samplerAddressMode;
			info.addressModeW = samplerAddressMode;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			info.minLod = 0;
			info.maxLod = mipLevels - 1;

			return info;
		}
		
		static VkWriteDescriptorSet writeDescriptorImage(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* imageInfo, uint32_t binding)
		{
			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstBinding = binding;
			write.dstSet = dstSet;
			write.descriptorCount = 1;
			write.descriptorType = type;
			write.pImageInfo = imageInfo;

			return write;
		}

		static unsigned int calculateMipLevels(const unsigned int width, const unsigned int height)
		{
			unsigned int mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
			return mipLevels;
		}

		static VkImageMemoryBarrier imageMemoryBarrier(VkImage image, VkImageAspectFlags aspectMask, int mipLevels, int layerCount)
		{
			VkImageMemoryBarrier imageBarrier{};
			imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageBarrier.image = image;
			imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBarrier.subresourceRange.baseArrayLayer = 0;
			imageBarrier.subresourceRange.layerCount = layerCount;
			imageBarrier.subresourceRange.levelCount = mipLevels;

			return imageBarrier;
		}

		static void transitionUndefinedToGeneral(VkCommandBuffer commandBuffer, VkImage image, int mipLevels, int layerCount)
		{
			VkImageMemoryBarrier imageBarrier = imageMemoryBarrier(image, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, layerCount);
			VkPipelineStageFlagBits srcFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			VkPipelineStageFlagBits dstFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			vkCmdPipelineBarrier(commandBuffer, srcFlags, dstFlags, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
		}

		static void transitionGeneralToShader(VkCommandBuffer commandBuffer, VkImage image, int mipLevels, int layerCount)
		{
			VkImageMemoryBarrier imageBarrier = imageMemoryBarrier(image, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, layerCount);
			VkPipelineStageFlagBits srcFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			VkPipelineStageFlagBits dstFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			vkCmdPipelineBarrier(commandBuffer, srcFlags, dstFlags, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
		}

		static void convertColorAttachmentToShaderRead(VkCommandBuffer commandBuffer, VmaAllocator allocator, AllocatedImage input, AllocatedImage& output, 
			VkExtent2D extent, int mipLevels, int layerCount)
		{
			//Create new image with shader read layout
			AllocatedImage newImage;
			VkImageCreateInfo newImageInfo = VkUtil::imageCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, extent, mipLevels);
			newImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			newImageInfo.arrayLayers = layerCount;
			VmaAllocationCreateInfo allocationInfo{};
			allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			vmaCreateImage(allocator, &newImageInfo, &allocationInfo, &newImage.image, &newImage.allocation, nullptr);

			//Transfer from color optimal to transfer source optimal
			VkImageMemoryBarrier colorToTransfer = imageMemoryBarrier(input.image, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, layerCount);
			colorToTransfer.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			colorToTransfer.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			colorToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &colorToTransfer);

			//Transfer from undefined to transfer destination optimal
			VkImageMemoryBarrier undefinedToTransfer = imageMemoryBarrier(newImage.image, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, layerCount);
			undefinedToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			undefinedToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			undefinedToTransfer.srcAccessMask = VK_ACCESS_NONE_KHR;
			undefinedToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &undefinedToTransfer);

			//Copy input image to output image
			uint32_t mipWidth = extent.width;
			uint32_t mipHeight = extent.height;
			std::vector<VkImageCopy> imageCopies;
			for (int i = 0; i < mipLevels; i++)
			{
				VkImageSubresourceLayers subresource{};
				subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				subresource.layerCount = layerCount;
				subresource.mipLevel = i;

				VkImageCopy imageCopy{};
				imageCopy.extent = { mipWidth, mipHeight, 1 };
				imageCopy.srcSubresource = subresource;
				imageCopy.dstSubresource = subresource;

				imageCopies.push_back(imageCopy);

				mipWidth /= 2;
				mipHeight /= 2;
			}
			vkCmdCopyImage(commandBuffer, input.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
				imageCopies.size(), imageCopies.data());

			//Transition new iamge to shader read optimal
			VkImageMemoryBarrier transferToShader = imageMemoryBarrier(newImage.image, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, layerCount);
			transferToShader.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			transferToShader.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			transferToShader.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			transferToShader.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &transferToShader);

			output = newImage;
		}

		static void generateMipMaps(VkCommandBuffer commandBuffer, VkImage oldImage, VkImage newImage, int width, int height, int layerCount)
		{
			const int mipLevels = calculateMipLevels(width, height);

			VkImageMemoryBarrier oldImageBarrier{};
			oldImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			oldImageBarrier.image = oldImage;
			oldImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			oldImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			oldImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			oldImageBarrier.subresourceRange.baseArrayLayer = 0;
			oldImageBarrier.subresourceRange.layerCount = layerCount;
			oldImageBarrier.subresourceRange.levelCount = 1;

			//Transition oldImage from current layout to transfer source
			oldImageBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			oldImageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			oldImageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			oldImageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &oldImageBarrier);

			//Transition newImage from undefined layout to transfer destination
			VkImageMemoryBarrier newImageBarrier = oldImageBarrier;
			newImageBarrier.image = newImage;
			newImageBarrier.subresourceRange.levelCount = mipLevels;
			newImageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			newImageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			newImageBarrier.srcAccessMask = VK_ACCESS_NONE_KHR;
			newImageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &newImageBarrier);

			//Generate mip maps
			int32_t mipWidth = width;
			int32_t mipHeight = height;
			for (int i = 0; i < mipLevels; i++)
			{
				VkImageBlit blit{};
				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = { width, height, 1 };
				blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.srcSubresource.mipLevel = 0;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 6;
				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = { mipWidth, mipHeight, 1 };
				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 6;

				vkCmdBlitImage(commandBuffer, oldImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, newImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

				if (mipWidth > 1) mipWidth /= 2;
				if (mipHeight > 1) mipHeight /= 2;
			}

			//Transition from transfer source to shader read only
			VkImageMemoryBarrier toShaderBarrier = newImageBarrier;
			toShaderBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			toShaderBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			toShaderBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			toShaderBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toShaderBarrier);
		}
	};
}