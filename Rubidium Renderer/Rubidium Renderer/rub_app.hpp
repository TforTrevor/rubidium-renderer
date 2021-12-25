#pragma once

#include "rub_window.hpp"
#include "rub_pipeline.hpp"
#include "rub_device.hpp"
#include "rub_swap_chain.hpp"
#include "rub_render_object.hpp"
#include "rub_renderer.hpp"
#include "rub_texture.hpp"
#include "rub_scene.hpp"

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