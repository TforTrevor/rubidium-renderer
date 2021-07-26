#pragma once

#include "rub_model.hpp"
#include "rub_material.hpp"

#include <memory>

namespace rub
{
	class RenderObject
	{
	public:
		RenderObject() {}
		~RenderObject() {}

		std::shared_ptr<Model> model{};
		glm::vec3 position;
		glm::vec3 rotation;
		std::shared_ptr<Material> material;
	};
}