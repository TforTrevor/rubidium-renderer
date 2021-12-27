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

		static void transitionImageColorToShader(VkCommandBuffer commandBuffer, VkImage image, int layerCount)
		{
			VkImageMemoryBarrier imageBarrier{};
			imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageBarrier.image = image;
			imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBarrier.subresourceRange.baseMipLevel = 0;
			imageBarrier.subresourceRange.levelCount = 1;
			imageBarrier.subresourceRange.layerCount = layerCount;
			imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			VkPipelineStageFlagBits srcFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			VkPipelineStageFlagBits dstFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			vkCmdPipelineBarrier(commandBuffer, srcFlags, dstFlags, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
		}

		static unsigned int calculateMipLevels(const unsigned int width, const unsigned int height)
		{
			unsigned int mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
			return mipLevels;
		}

		static void generateMipMaps(Device& device, VkImage image, int width, int height, int mipLevels, int layerCount)
		{
			VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image = image;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = layerCount;
			barrier.subresourceRange.levelCount = 1;

			int32_t mipWidth = width;
			int32_t mipHeight = height;

			for (int i = 1; i < mipLevels; i++)
			{
				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

				VkImageBlit blit{};
				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
				blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = layerCount;
				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = layerCount;

				vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

				if (mipWidth > 1) mipWidth /= 2;
				if (mipHeight > 1) mipHeight /= 2;
			}

			barrier.subresourceRange.baseMipLevel = mipLevels - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			device.endSingleTimeCommands(commandBuffer);
		}
	};
}