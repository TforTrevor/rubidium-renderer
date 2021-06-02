#pragma once

#include "vulkan/vulkan.h"

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


	};
}