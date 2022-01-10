#include "app.hpp"

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
		std::shared_ptr<Camera> camera = std::make_shared<Camera>(window, 70);
		camera->setPosition(glm::vec3(0, 0, -3.0f));

		scene = std::make_unique<Scene>(device, renderer.getSwapChain(), camera, "textures/spruit_sunrise_2k.exr", renderObjects);

		VkRenderPass renderPass = renderer.getRenderPass();

		double lastTime = glfwGetTime();
		int nbFrames = 0;

		while (!window.shouldClose())
		{
			glfwPollEvents();

			camera->updatePosition(window);
			camera->updateRotation(window);

			double cpuTime = 0;

			auto commandBuffer = renderer.beginFrame();
			if (commandBuffer != nullptr)
			{
				using namespace std::chrono;
				
				auto cpuTime1 = high_resolution_clock::now();
				renderer.beginRenderPass(commandBuffer);
				scene->draw(commandBuffer, renderPass);
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
		//std::shared_ptr<Model> suzanne = std::make_shared<Model>(device, "models/suzanne_2.obj");
		//std::shared_ptr<Model> triangle = std::make_shared<Model>(device, "models/triangle.obj");
		//std::shared_ptr<Model> cube = std::make_shared<Model>(device, "models/cube.obj");
		std::shared_ptr<Model> sphere = std::make_shared<Model>(device, "models/sphere.obj");

		std::shared_ptr<Texture> blueWallAlbedo = std::make_shared<Texture>(device, "textures/PaintedBricks001_1K_Color.png", Texture::Format::SRGB);
		std::shared_ptr<Texture> blueWallNormal = std::make_shared<Texture>(device, "textures/PaintedBricks001_1K_Normal.png", Texture::Format::LINEAR);
		std::shared_ptr<Texture> blueWallMask = std::make_shared<Texture>(device, "textures/PaintedBricks001_1K_Mask.png", Texture::Format::LINEAR);

		//std::shared_ptr<Texture> brickWallAlbedo = std::make_shared<Texture>(device, "textures/Bricks071_1K_Color.png", Texture::Format::SRGB);
		//std::shared_ptr<Texture> brickWallNormal = std::make_shared<Texture>(device, "textures/Bricks071_1K_Normal.png", Texture::Format::LINEAR);
		//std::shared_ptr<Texture> brickWallMask = std::make_shared<Texture>(device, "textures/Bricks071_1K_Roughness.png", Texture::Format::LINEAR);

		std::shared_ptr<Texture> metalAlbedo = std::make_shared<Texture>(device, "textures/Metal011_1K_Color.png", Texture::Format::SRGB);
		std::shared_ptr<Texture> metalNormal = std::make_shared<Texture>(device, "textures/Metal011_1K_NormalGL.png", Texture::Format::LINEAR);
		std::shared_ptr<Texture> metalMask = std::make_shared<Texture>(device, "textures/Metal011_1K_Mask.png", Texture::Format::LINEAR);

		std::shared_ptr<Material> blueWallMaterial = std::make_shared<Material>(device, "shaders/pbr.vert.spv", "shaders/pbr.frag.spv");
		blueWallMaterial->addTexture(blueWallAlbedo);
		blueWallMaterial->addTexture(blueWallNormal);
		blueWallMaterial->addTexture(blueWallMask);
		//std::shared_ptr<Material> brickWallMaterial = std::make_shared<Material>(device, brickWallAlbedo, brickWallNormal, brickWallMask);
		std::shared_ptr<Material> metalMaterial = std::make_shared<Material>(device, "shaders/pbr.vert.spv", "shaders/pbr.frag.spv");
		metalMaterial->addTexture(metalAlbedo);
		metalMaterial->addTexture(metalNormal);
		metalMaterial->addTexture(metalMask);

		RenderObject object{};
		object.model = sphere;
		object.transform = Transform{ glm::vec3(-1.5f, 0, 0), glm::vec3(0, 0.0f, 0) };
		object.material = blueWallMaterial;

		RenderObject object2{};
		object2.model = sphere;
		object2.transform = Transform{ glm::vec3(1.5f, 0, 0), glm::vec3(0, 180.0f, 0) };
		object2.material = metalMaterial;

		////RenderObject object3{};
		////object3.model = cube;
		////object3.position = glm::vec3(-1.5f, 0, 0);
		////object3.rotation = glm::vec3(0, glm::radians(180.0f), 0);
		////object3.material = RenderObject::Material{ glm::vec4(1.0f, 1.0f, 0.5f, 1), glm::vec4(0, 1, 0, 0) };

		////RenderObject object4{};
		////object4.model = cube;
		////object4.position = glm::vec3(1.5f, 0, 0);
		////object4.rotation = glm::vec3(0, glm::radians(180.0f), 0);
		////object4.material = RenderObject::Material{ glm::vec4(0.5f, 1.0f, 1.0f, 1), glm::vec4(0, 1, 0, 0) };

		renderObjects.push_back(std::move(object));
		renderObjects.push_back(std::move(object2));
		////gameObjects.push_back(std::move(object3));
		////gameObjects.push_back(std::move(object4));

		////int objectCount = 10000;

		////for (int i = 0; i < objectCount; i++)
		////{
		////	RenderObject obj{};
		////	obj.model = triangle;
		////	obj.transform = Transform{ glm::vec3(0, -1.0f, 0), glm::vec3(0, 0.0f, 0) };
		////	obj.material = blueWallMaterial;
		////	renderObjects.push_back(std::move(obj));
		////}
	}

	RubApp::~RubApp()
	{

	}
}