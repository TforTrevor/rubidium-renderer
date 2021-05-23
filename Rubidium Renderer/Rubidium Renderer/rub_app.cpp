#include "rub_app.hpp"
#include "simple_render_system.hpp"

#include <stdexcept>
#include <array>

namespace rub
{
	RubApp::RubApp()
	{
		loadModels();
	}

	void RubApp::run()
	{
		SimpleRenderSystem renderSystem{ rubDevice, rubRenderer.getSwapChain(), rubRenderer.getRenderPass() };

		while (!rubWindow.shouldClose())
		{
			glfwPollEvents();

			auto commandBuffer = rubRenderer.beginFrame();
			if (commandBuffer != nullptr)
			{
				rubRenderer.beginRenderPass(commandBuffer);
				renderSystem.renderModels(commandBuffer, gameObjects);
				rubRenderer.endRenderPass(commandBuffer);
				rubRenderer.endFrame();
			}			
		}

		vkDeviceWaitIdle(rubDevice.getDevice());
	}

	void RubApp::loadModels()
	{
		//std::vector<RubModel::Vertex> vertices
		//{
		//	{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		//	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		//	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		//};

		const std::vector<RubModel::Vertex> vertices = {
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};

		const std::vector<uint32_t> indices = {
			0, 1, 2, 2, 3, 0
		};

		auto object = RubGameObject::createGameObject();
		object.model = std::make_shared<RubModel>(rubDevice, vertices, indices);

		gameObjects.push_back(std::move(object));
	}

	RubApp::~RubApp()
	{

	}
}