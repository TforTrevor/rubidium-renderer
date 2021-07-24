#pragma once

#include "rub_model.hpp"

#include <memory>

namespace rub
{
	class RenderObject
	{
	public:
		struct Material
		{
			glm::vec4 albedo;
			glm::vec4 maskMap; //r: metallic, g: roughness
		};

		RenderObject() {}
		~RenderObject() {}

		std::shared_ptr<Model> model{};
		glm::vec3 position;
		glm::vec3 rotation;
		Material material;
	};
}