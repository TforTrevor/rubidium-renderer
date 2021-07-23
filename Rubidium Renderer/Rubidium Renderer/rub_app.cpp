#include "rub_app.hpp"
#include "simple_render_system.hpp"

#include <stdexcept>
#include <array>
#include <sstream>
#include <iomanip>

namespace rub
{
	RubApp::RubApp()
	{
		loadObjects();
	}

	void RubApp::run()
	{
		SimpleRenderSystem renderSystem{ rubDevice, rubRenderer.getRenderPass(), rubRenderer.getGlobalDescriptor(), rubRenderer.getSwapChain() };

		double lastTime = glfwGetTime();
		int nbFrames = 0;

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

			double currentTime = glfwGetTime();
			nbFrames++;
			if (currentTime - lastTime >= 1.0)
			{ // If last prinf() was more than 1 sec ago
			  // printf and reset timer
				//printf("%f ms/frame\n", 1000.0 / double(nbFrames));
				std::stringstream suffix;
				suffix << std::fixed << std::setprecision(3) << 1000.0 / double(nbFrames) << "ms";
				rubWindow.changeTitleSuffix(suffix.str());

				nbFrames = 0;
				lastTime += 1.0;				
			}
		}

		vkDeviceWaitIdle(rubDevice.getDevice());
	}

	void RubApp::loadObjects()
	{
		//std::shared_ptr<RubModel> suzanne = std::make_shared<RubModel>(rubDevice, "models/suzanne_2.obj");
		std::shared_ptr<RubModel> triangle = std::make_shared<RubModel>(rubDevice, "models/triangle.obj");

		//RubGameObject object{};
		//object.model = suzanne;
		//object.position = glm::vec3(-1.5f, 0, 0);
		//object.rotation = glm::vec3(0, glm::radians(0.0f), 0);
		//object.material = RubGameObject::Material{ glm::vec4(1.0f, 1.0f, 1.0f, 1), glm::vec4(0, 1, 0, 0) };

		//RubGameObject object2{};
		//object2.model = suzanne;
		//object2.position = glm::vec3(1.5f, 0, 0);
		//object2.rotation = glm::vec3(0, glm::radians(180.0f), 0);
		//object2.material = RubGameObject::Material{ glm::vec4(1.0f, 0.5f, 0.5f, 1), glm::vec4(0, 1, 0, 0) };

		//gameObjects.push_back(std::move(object));
		//gameObjects.push_back(std::move(object2));

		int objectCount = 10000;

		for (int i = 0; i < objectCount; i++)
		{
			RubGameObject obj{};
			obj.model = triangle;
			obj.position = glm::vec3(0, -1, 0);
			gameObjects.push_back(std::move(obj));
		}
	}

	RubApp::~RubApp()
	{

	}
}