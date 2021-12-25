#pragma once

#include "rub_render_object.hpp"

namespace rub
{
	class Skybox
	{
	public:
		Skybox(Device& device, const std::string& environmentPath);
		~Skybox();

		void bind();

	private:
		Device& device;

		std::shared_ptr<Model> skyboxModel;

		void equiToCube(const std::string& environmentPath);
	};
}