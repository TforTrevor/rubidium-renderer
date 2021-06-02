#pragma once

#include "rub_model.hpp"

#include <memory>

namespace rub
{
	class RubGameObject
	{
	public:
		static RubGameObject createGameObject()
		{
			//return RubGameObject{ currentId++ };
			return RubGameObject{};
		}

		RubGameObject() {}
		~RubGameObject() {}

		std::shared_ptr<RubModel> model{};
		glm::vec3 position;
		glm::vec3 rotation;
	private:
		//static uint32_t currentId;
		//uint32_t id;
	};
}