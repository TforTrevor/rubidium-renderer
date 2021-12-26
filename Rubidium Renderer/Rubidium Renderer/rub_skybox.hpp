#pragma once

#include "rub_render_object.hpp"
#include "rub_cubemap.hpp"

namespace rub
{
	class Skybox
	{
	public:
		Skybox(Device& device, const std::string& environmentPath);
		~Skybox();

		void draw(VkCommandBuffer commandBuffer);
		std::shared_ptr<Material> getMaterial() { return skyboxObject.material; }

	private:
		Device& device;

		std::shared_ptr<Model> skyboxModel;
		std::unique_ptr<Cubemap> cubemap;
		RenderObject skyboxObject{};

		void equiToCube(const std::string& environmentPath);
	};
}