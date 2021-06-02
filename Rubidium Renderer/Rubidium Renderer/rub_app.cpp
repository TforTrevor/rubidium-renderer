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
		SimpleRenderSystem renderSystem{ rubDevice, rubRenderer.getRenderPass(), rubRenderer.getGlobalDescriptor() };

		while (!rubWindow.shouldClose())
		{
			glfwPollEvents();

			auto commandBuffer = rubRenderer.beginFrame();
			if (commandBuffer != nullptr)
			{
				rubRenderer.beginRenderPass(commandBuffer);
				renderSystem.renderModels(commandBuffer, gameObjects, rubRenderer.getGlobalDescriptor());
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

		//const std::vector<RubModel::Vertex> vertices = {
		//	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		//	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		//	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		//	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		//};

		//const std::vector<uint32_t> indices = {
		//	0, 1, 2, 2, 3, 0
		//};

		std::shared_ptr<RubModel> suzanne = std::make_shared<RubModel>(rubDevice, "models/suzanne_2.obj");

		auto object = RubGameObject::createGameObject();
		object.model = suzanne;
		object.position = glm::vec3(-1.5f, 0, 0);

		auto object2 = RubGameObject::createGameObject();
		object2.model = suzanne;
		object2.position = glm::vec3(1.5f, 0, 0);
		object2.rotation = glm::vec3(0, glm::radians(180.0f), 0);

		gameObjects.push_back(std::move(object));
		gameObjects.push_back(std::move(object2));
	}

	RubApp::~RubApp()
	{

	}
}