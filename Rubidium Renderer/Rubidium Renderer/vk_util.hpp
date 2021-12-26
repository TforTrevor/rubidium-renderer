#pragma once

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

		static VkImageCreateInfo imageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent2D extent)
		{
			VkImageCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			info.pNext = nullptr;
			info.imageType = VK_IMAGE_TYPE_2D;
			info.format = format;
			info.extent = { extent.width, extent.height, 1 };
			info.mipLevels = 1;
			info.arrayLayers = 1;
			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.tiling = VK_IMAGE_TILING_OPTIMAL;
			info.usage = usageFlags;

			return info;
		}

		static VkImageViewCreateInfo imageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
		{
			VkImageViewCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.pNext = nullptr;
			info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.image = image;
			info.format = format;
			info.subresourceRange.baseMipLevel = 0;
			info.subresourceRange.levelCount = 1;
			info.subresourceRange.baseArrayLayer = 0;
			info.subresourceRange.layerCount = 1;
			info.subresourceRange.aspectMask = aspectFlags;

			return info;
		}

		static VkSamplerCreateInfo samplesCreateInfo(VkFilter filters, VkSamplerAddressMode samplerAddressMode /*= VK_SAMPLER_ADDRESS_MODE_REPEAT*/)
		{
			VkSamplerCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.pNext = nullptr;

			info.magFilter = filters;
			info.minFilter = filters;
			info.addressModeU = samplerAddressMode;
			info.addressModeV = samplerAddressMode;
			info.addressModeW = samplerAddressMode;

			return info;
		}
		
		static VkWriteDescriptorSet writeDescriptorImage(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* imageInfo, uint32_t binding)
		{
			VkWriteDescriptorSet write = {};
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
	};
}