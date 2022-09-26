#include "model.hpp"

#include <rapidobj/rapidobj.hpp>

#include <cassert>
#include <stdexcept>
#include <unordered_map>
#include <iostream>

#include "vk_util.hpp"

namespace rub
{
	Model::Model(Device& device, const std::string modelPath) : device{ device }
	{
		loadOBJ(modelPath);
	}

	void Model::loadOBJ(const std::string& modelPath)
	{
		rapidobj::Result objResult = rapidobj::ParseFile(modelPath);
		rapidobj::Triangulate(objResult);

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : objResult.shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};

				vertex.position = {
					objResult.attributes.positions[3 * index.position_index + 0],
					objResult.attributes.positions[3 * index.position_index + 1],
					objResult.attributes.positions[3 * index.position_index + 2]
				};

				vertex.normal = {
					objResult.attributes.normals[3 * index.normal_index + 0],
					objResult.attributes.normals[3 * index.normal_index + 1],
					objResult.attributes.normals[3 * index.normal_index + 2]
				};

				vertex.texCoord = {
					objResult.attributes.texcoords[2 * index.texcoord_index + 0],
					objResult.attributes.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.color = {
					/*attrib.colors[3 * index.vertex_index + 0],
					attrib.colors[3 * index.vertex_index + 1],
					attrib.colors[3 * index.vertex_index + 2]*/
					//attrib.normals[3 * index.normal_index + 0],
					//attrib.normals[3 * index.normal_index + 1],
					//attrib.normals[3 * index.normal_index + 2]
					1, 1, 1
				};

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}

		createVertexBuffer(vertices);
		createIndexBuffer(indices);

		std::cout << "OBJ File: " << modelPath << std::endl;
		std::cout << "\tVertex count: " << vertices.size() << std::endl;
		std::cout << "\tIndex count: " << indices.size() << std::endl;
	}

	void Model::createVertexBuffer(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

		AllocatedBuffer stagingBuffer;
		device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

		void* data;
		vmaMapMemory(device.getAllocator(), stagingBuffer.allocation, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vmaUnmapMemory(device.getAllocator(), stagingBuffer.allocation);

		device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, vertexBuffer);

		VkUtil::copyBuffer(device, stagingBuffer.buffer, vertexBuffer.buffer, bufferSize);

		vmaDestroyBuffer(device.getAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);
	}

	void Model::createIndexBuffer(const std::vector<uint32_t>& indices)
	{
		indexCount = static_cast<uint32_t>(indices.size());
		assert(indexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

		AllocatedBuffer stagingBuffer;
		device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

		void* data;
		vmaMapMemory(device.getAllocator(), stagingBuffer.allocation, &data);
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vmaUnmapMemory(device.getAllocator(), stagingBuffer.allocation);

		device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, indexBuffer);

		VkUtil::copyBuffer(device, stagingBuffer.buffer, indexBuffer.buffer, bufferSize);

		vmaDestroyBuffer(device.getAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);
	}

	void Model::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer.buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void Model::draw(VkCommandBuffer commandBuffer, uint32_t firstInstance)
	{
		//vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, firstInstance);
	}

	std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}

	Model::~Model()
	{
		vmaDestroyBuffer(device.getAllocator(), vertexBuffer.buffer, vertexBuffer.allocation);
		vmaDestroyBuffer(device.getAllocator(), indexBuffer.buffer, indexBuffer.allocation);
	}
}