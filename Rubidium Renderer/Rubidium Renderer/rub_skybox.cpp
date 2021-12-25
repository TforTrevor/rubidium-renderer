#include "rub_skybox.hpp"

#include "rub_cubemap.hpp"

namespace rub
{
	Skybox::Skybox(Device& device, const std::string& environmentPath) : device{ device }
	{
		skyboxModel = std::make_shared<Model>(device, "models/cube.obj");
		equiToCube(environmentPath);
	}

	void Skybox::equiToCube(const std::string& environmentPath)
	{
		std::shared_ptr<Texture> equiTexture = std::make_shared<Texture>(device, environmentPath, Texture::Format::HDR);
		std::shared_ptr<Material> equiMaterial = std::make_shared<Material>(device, "shaders/skybox.vert.spv", "shaders/equi_to_cube.frag.spv");
		equiMaterial->addTexture(equiTexture);

		RenderObject equiObject{};
		equiObject.model = skyboxModel;
		equiObject.transform = { glm::vec3(0), glm::vec3(0) };
		equiObject.material = equiMaterial;

		std::vector<RenderObject> renderObjects = { equiObject };

		Cubemap cubemap{ device };
		cubemap.capture(renderObjects);
	}

	Skybox::~Skybox()
	{

	}
}