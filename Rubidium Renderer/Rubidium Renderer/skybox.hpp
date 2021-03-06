#pragma once

#include "render_object.hpp"
#include "cubemap.hpp"

namespace rub
{
	class Skybox
	{
	public:
		Skybox(Device& device, const std::string& environmentPath);
		~Skybox();

		void draw(VkCommandBuffer commandBuffer);
		std::shared_ptr<Material> getMaterial() { return skyboxObject.material; }
		VkImageView getIrradiance() { return cubemap->getIrradianceImageView(); }
		VkImageView getPrefilter() { return cubemap->getPrefilterImageView(); }
		int getPrefilterMipLevels() { return cubemap->getCaptureMipLevels(); }

	private:
		Device& device;

		std::shared_ptr<Model> skyboxModel;
		std::unique_ptr<Cubemap> cubemap;
		RenderObject skyboxObject{};

		void equiToCube(const std::string& environmentPath);
	};
}