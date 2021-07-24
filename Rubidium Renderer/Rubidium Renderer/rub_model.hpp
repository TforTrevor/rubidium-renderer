#pragma once

#include "rub_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>
#include <iostream>

namespace rub
{
	class RubModel
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

		RubModel(RubDevice& rubDevice, const std::string modelPath);
		~RubModel();

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer, uint32_t firstInstance);

	private:
		RubDevice& rubDevice;

		AllocatedBuffer vertexBuffer;
		uint32_t vertexCount;

		AllocatedBuffer indexBuffer;
		uint32_t indexCount;

		void loadOBJ(const std::string modelPath);
		void createVertexBuffer(const std::vector<Vertex>& vertices);
		void createIndexBuffer(const std::vector<uint32_t>& indices);
	};
}

namespace std
{
	template<> struct hash<rub::RubModel::Vertex>
	{
		size_t operator()(rub::RubModel::Vertex const& vertex) const
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