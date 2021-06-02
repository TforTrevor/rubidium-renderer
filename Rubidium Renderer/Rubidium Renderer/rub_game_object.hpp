#pragma once

#include "rub_model.hpp"

#include <memory>

namespace rub
{
	class RubGameObject
	{
	public:
		struct Material
		{
			glm::vec4 albedo;
			glm::vec4 maskMap; //r: metallic, g: roughness
		};

		RubGameObject() {}
		~RubGameObject() {}

		std::shared_ptr<RubModel> model{};
		glm::vec3 position;
		glm::vec3 rotation;
		Material material;
	};
}