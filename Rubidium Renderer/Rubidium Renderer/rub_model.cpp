#include "rub_model.hpp"
#include "mvp_matrix.hpp"

#include <cassert>

namespace rub
{
	RubModel::RubModel(RubDevice& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) : rubDevice{ device }
	{
		createVertexBuffer(vertices);
		createIndexBuffer(indices);
	}

	void RubModel::createVertexBuffer(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		rubDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(rubDevice.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(rubDevice.getDevice(), stagingBufferMemory);

		rubDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		rubDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(rubDevice.getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(rubDevice.getDevice(), stagingBufferMemory, nullptr);
	}

	void RubModel::createIndexBuffer(const std::vector<uint32_t>& indices)
	{
		indexCount = static_cast<uint32_t>(indices.size());
		assert(indexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		rubDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(rubDevice.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(rubDevice.getDevice(), stagingBufferMemory);

		rubDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		rubDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(rubDevice.getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(rubDevice.getDevice(), stagingBufferMemory, nullptr);
	}

	void RubModel::createUniformBuffers(int imageCount)
	{
		for (int i = 0; i < uniformBuffers.size(); i++)
		{
			vkDestroyBuffer(rubDevice.getDevice(), uniformBuffers[i], nullptr);
			vkFreeMemory(rubDevice.getDevice(), uniformBuffersMemory[i], nullptr);
		}

		VkDeviceSize bufferSize = sizeof(MVPMatrix);

		uniformBuffers.resize(imageCount);
		uniformBuffersMemory.resize(imageCount);

		for (size_t i = 0; i < imageCount; i++)
		{
			rubDevice.createBuffer(
				bufferSize, 
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				uniformBuffers[i], 
				uniformBuffersMemory[i]);
		}
	}

	void RubModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void RubModel::draw(VkCommandBuffer commandBuffer)
	{
		//vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
	}

	std::vector<VkVertexInputBindingDescription> RubModel::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> RubModel::Vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		return attributeDescriptions;
	}

	RubModel::~RubModel()
	{
		vkDestroyBuffer(rubDevice.getDevice(), vertexBuffer, nullptr);
		vkFreeMemory(rubDevice.getDevice(), vertexBufferMemory, nullptr);

		vkDestroyBuffer(rubDevice.getDevice(), indexBuffer, nullptr);
		vkFreeMemory(rubDevice.getDevice(), indexBufferMemory, nullptr);

		for (int i = 0; i < uniformBuffers.size(); i++)
		{
			vkDestroyBuffer(rubDevice.getDevice(), uniformBuffers[i], nullptr);
			vkFreeMemory(rubDevice.getDevice(), uniformBuffersMemory[i], nullptr);
		}
	}
}