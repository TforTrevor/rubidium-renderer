#pragma once

#include "device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>
#include <iostream>

namespace rub
{
	class Model
	{
	public:

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 texCoord;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Vertex& other) const
			{
				return position == other.position && color == other.color && texCoord == other.texCoord;
			}
		};

		Model(Device& rubDevice, const std::string modelPath);
		~Model();

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer, uint32_t firstInstance);

	private:
		Device& device;

		AllocatedBuffer vertexBuffer;
		uint32_t vertexCount;

		AllocatedBuffer indexBuffer;
		uint32_t indexCount;

		void loadOBJ(const std::string& modelPath);
		void createVertexBuffer(const std::vector<Vertex>& vertices);
		void createIndexBuffer(const std::vector<uint32_t>& indices);
	};
}

namespace std
{
	template<> struct hash<rub::Model::Vertex>
	{
		size_t operator()(rub::Model::Vertex const& vertex) const
		{
			//return 1;
			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
			//return (
			//	hash<glm::vec3>()(vertex.position) ^ 
			//	hash<glm::vec3>()(vertex.normal) ^
			//	hash<glm::vec2>()(vertex.texCoord) ^
			//	hash<glm::vec3>()(vertex.color)
			//);
		}
	};
}