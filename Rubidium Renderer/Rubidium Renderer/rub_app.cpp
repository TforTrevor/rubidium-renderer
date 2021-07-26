#include "rub_app.hpp"
#include "simple_render_system.hpp"

#include <stdexcept>
#include <array>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace rub
{
	RubApp::RubApp()
	{
		loadObjects();
	}

	void RubApp::run()
	{
		VkRenderPass renderPass = renderer.getRenderPass();
		SimpleRenderSystem renderSystem{ device, renderer.getSwapChain() };

		double lastTime = glfwGetTime();
		int nbFrames = 0;

		while (!window.shouldClose())
		{
			glfwPollEvents();

			double cpuTime = 0;

			auto commandBuffer = renderer.beginFrame();
			if (commandBuffer != nullptr)
			{
				using namespace std::chrono;
				
				auto cpuTime1 = high_resolution_clock::now();
				renderer.beginRenderPass(commandBuffer);
				renderSystem.renderModels(commandBuffer, renderObjects, renderer.getGlobalDescriptor(), renderPass);
				renderer.endRenderPass(commandBuffer);
				auto cpuTime2 = high_resolution_clock::now();
				duration<double, std::milli> cpuTimeMs = cpuTime2 - cpuTime1;
				cpuTime = cpuTimeMs.count();

				renderer.endFrame();
			}

			double currentTime = glfwGetTime();
			nbFrames++;
			if (currentTime - lastTime >= 1.0)
			{ // If last prinf() was more than 1 sec ago
			  // printf and reset timer
				//printf("%f ms/frame\n", 1000.0 / double(nbFrames));
				std::stringstream suffix;
				suffix << std::fixed << std::setprecision(3) << 1000.0 / double(nbFrames) << "ms - CPU Time: " << cpuTime << "ms";
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
		std::shared_ptr<Model> sphere = std::make_shared<Model>(device, "models/sphere.obj");

		std::shared_ptr<Texture> blueWallAlbedo = std::make_shared<Texture>(device, "textures/PaintedBricks001_1K_Color.png", VK_FORMAT_R8G8B8A8_SRGB);
		std::shared_ptr<Texture> blueWallNormal = std::make_shared<Texture>(device, "textures/PaintedBricks001_1K_Normal.png", VK_FORMAT_R8G8B8A8_UNORM);
		std::shared_ptr<Texture> blueWallMask = std::make_shared<Texture>(device, "textures/PaintedBricks001_1K_Roughness.png", VK_FORMAT_R8G8B8A8_UNORM);

		std::shared_ptr<Texture> brickWallAlbedo = std::make_shared<Texture>(device, "textures/Bricks071_1K_Color.png", VK_FORMAT_R8G8B8A8_SRGB);
		std::shared_ptr<Texture> brickWallNormal = std::make_shared<Texture>(device, "textures/Bricks071_1K_Normal.png", VK_FORMAT_R8G8B8A8_UNORM);
		std::shared_ptr<Texture> brickWallMask = std::make_shared<Texture>(device, "textures/Bricks071_1K_Roughness.png", VK_FORMAT_R8G8B8A8_UNORM);

		std::shared_ptr<Material> blueWallMaterial = std::make_shared<Material>(device, blueWallAlbedo, blueWallNormal, blueWallMask);
		std::shared_ptr<Material> brickWallMaterial = std::make_shared<Material>(device, brickWallAlbedo, brickWallNormal, brickWallMask);

		RenderObject object{};
		object.model = sphere;
		object.transform = Transform{ glm::vec3(-1.5f, 0, 0), glm::vec3(0, 0.0f, 0) };
		object.material = blueWallMaterial;

		RenderObject object2{};
		object2.model = sphere;
		object2.transform = Transform{ glm::vec3(1.5f, 0, 0), glm::vec3(0, 180.0f, 0) };
		object2.material = blueWallMaterial;

		//RenderObject object3{};
		//object3.model = cube;
		//object3.position = glm::vec3(-1.5f, 0, 0);
		//object3.rotation = glm::vec3(0, glm::radians(180.0f), 0);
		//object3.material = RenderObject::Material{ glm::vec4(1.0f, 1.0f, 0.5f, 1), glm::vec4(0, 1, 0, 0) };

		//RenderObject object4{};
		//object4.model = cube;
		//object4.position = glm::vec3(1.5f, 0, 0);
		//object4.rotation = glm::vec3(0, glm::radians(180.0f), 0);
		//object4.material = RenderObject::Material{ glm::vec4(0.5f, 1.0f, 1.0f, 1), glm::vec4(0, 1, 0, 0) };

		//renderObjects.push_back(std::move(object));
		//renderObjects.push_back(std::move(object2));
		//gameObjects.push_back(std::move(object3));
		//gameObjects.push_back(std::move(object4));

		int objectCount = 10000;

		for (int i = 0; i < objectCount; i++)
		{
			RenderObject obj{};
			obj.model = triangle;
			obj.transform = Transform{ glm::vec3(0, -1.0f, 0), glm::vec3(0, 0.0f, 0) };
			obj.material = blueWallMaterial;
			renderObjects.push_back(std::move(obj));
		}
	}

	RubApp::~RubApp()
	{

	}
}