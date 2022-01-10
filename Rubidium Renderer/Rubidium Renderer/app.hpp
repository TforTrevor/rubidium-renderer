#pragma once

#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "render_object.hpp"
#include "renderer.hpp"
#include "texture.hpp"
#include "scene.hpp"

#include <memory>
#include <vector>
#include <string>

namespace rub
{
	class RubApp
	{
	public:
		static constexpr int WIDTH = 1280;
		static constexpr int HEIGHT = 720;

		RubApp();
		~RubApp();

		void run();

	private:
		Window window{ WIDTH, HEIGHT, "Rubidium Renderer" };
		Device device{ window };
		Renderer renderer{ window, device };

		std::vector<RenderObject> renderObjects;
		std::unique_ptr<Scene> scene;

		void loadObjects();
	};
}