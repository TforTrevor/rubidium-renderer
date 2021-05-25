#include "rub_model.hpp"
#include "mvp_matrix.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <cassert>
#include <stdexcept>
#include <unordered_map>
#include <iostream>

namespace rub
{
	RubModel::RubModel(RubDevice& device, const std::string modelPath) : rubDevice{ device }
	{
		loadOBJ(modelPath);
	}

	void RubModel::loadOBJ(const std::string modelPath)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str()))
		{
			throw std::runtime_error(warn + err);
		}

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};

				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.color = {
					attrib.colors[3 * index.vertex_index + 0],
					attrib.colors[3 * index.vertex_index + 1],
					attrib.colors[3 * index.vertex_index + 2]
					//attrib.normals[3 * index.normal_index + 0],
					//attrib.normals[3 * index.normal_index + 1],
					//attrib.normals[3 * index.normal_index + 2]
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

		std::cout << "Vertex count: " << vertices.size() << std::endl;
		std::cout << "Index count: " << indices.size() << std::endl;
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