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

		//RubGameObject(uint32_t id) : id{ id } {}
		//~RubGameObject() {}

		std::shared_ptr<RubModel> model{};
		glm::mat4 modelMatrix{};
	private:
		//static uint32_t currentId;
		//uint32_t id;
	};
}