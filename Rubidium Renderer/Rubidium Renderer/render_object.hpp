#pragma once

#include "model.hpp"
#include "material.hpp"
#include "transform.hpp"

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