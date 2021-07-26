#pragma once

#include "rub_model.hpp"
#include "rub_material.hpp"
#include "rub_transform.hpp"

#include <memory>

namespace rub
{
	class RenderObject
	{
	public:
		RenderObject() {}
		~RenderObject() {}

		std::shared_ptr<Model> model{};
		std::shared_ptr<Material> material;
		Transform transform;
	};
}