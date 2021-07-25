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
		loadTextures();
	}

	void RubApp::run()
	{
		SimpleRenderSystem renderSystem{ device, renderer.getRenderPass(), renderer.getGlobalDescriptor(), renderer.getSwapChain(), texture };

		double lastTime = glfwGetTime();
		int nbFrames = 0;

		while (!window.shouldClose())
		{
			glfwPollEvents();

			auto commandBuffer = renderer.beginFrame();
			if (commandBuffer != nullptr)
			{
				renderer.beginRenderPass(commandBuffer);
				renderSystem.renderModels(commandBuffer, renderObjects, renderer.getGlobalDescriptor());
				renderer.endRenderPass(commandBuffer);
				renderer.endFrame();
			}

			double currentTime = glfwGetTime();
			nbFrames++;
			if (currentTime - lastTime >= 1.0)
			{ // If last prinf() was more than 1 sec ago
			  // printf and reset timer
				//printf("%f ms/frame\n", 1000.0 / double(nbFrames));
				std::stringstream suffix;
				suffix << std::fixed << std::setprecision(3) << 1000.0 / double(nbFrames) << "ms";
				window.changeTitleSuffix(suffix.str());

				nbFrames = 0;
				lastTime += 1.0;				
			}
		}

		vkDeviceWaitIdle(device.getDevice());
	}

	void RubApp::loadObjects()
	{
		std::shared_ptr<Model> suzanne = std::make_shared<Model>(device, "models/suzanne_2.obj");
		std::shared_ptr<Model> triangle = std::make_shared<Model>(device, "models/triangle.obj");
		std::shared_ptr<Model> cube = std::make_shared<Model>(device, "models/cube.obj");

		RenderObject object{};
		object.model = suzanne;
		object.position = glm::vec3(-1.5f, 0, 0);
		object.rotation = glm::vec3(0, glm::radians(0.0f), 0);
		object.material = RenderObject::Material{ glm::vec4(1.0f, 1.0f, 1.0f, 1), glm::vec4(0, 1, 0, 0) };

		RenderObject object2{};
		object2.model = suzanne;
		object2.position = glm::vec3(1.5f, 0, 0);
		object2.rotation = glm::vec3(0, glm::radians(180.0f), 0);
		object2.material = RenderObject::Material{ glm::vec4(1.0f, 0.5f, 0.5f, 1), glm::vec4(0, 1, 0, 0) };

		RenderObject object3{};
		object3.model = cube;
		object3.position = glm::vec3(-1.5f, 0, 0);
		object3.rotation = glm::vec3(0, glm::radians(180.0f), 0);
		object3.material = RenderObject::Material{ glm::vec4(1.0f, 1.0f, 0.5f, 1), glm::vec4(0, 1, 0, 0) };

		RenderObject object4{};
		object4.model = cube;
		object4.position = glm::vec3(1.5f, 0, 0);
		object4.rotation = glm::vec3(0, glm::radians(180.0f), 0);
		object4.material = RenderObject::Material{ glm::vec4(0.5f, 1.0f, 1.0f, 1), glm::vec4(0, 1, 0, 0) };

		renderObjects.push_back(std::move(object));
		renderObjects.push_back(std::move(object2));
		//gameObjects.push_back(std::move(object3));
		//gameObjects.push_back(std::move(object4));

		//int objectCount = 10000;

		//for (int i = 0; i < objectCount; i++)
		//{
		//	RubGameObject obj{};
		//	obj.model = triangle;
		//	obj.position = glm::vec3(0, -1, 0);
		//	gameObjects.push_back(std::move(obj));
		//}
	}

	void RubApp::loadTextures()
	{
		std::shared_ptr<Texture> blueWall = std::make_shared<Texture>(device, "textures/PaintedBricks001_1K_Color.png");
		std::shared_ptr<Texture> brickWall = std::make_shared<Texture>(device, "textures/Bricks071_1K_Color.png");
		texture = brickWall;
	}

	RubApp::~RubApp()
	{

	}
}