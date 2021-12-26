#include "rub_skybox.hpp"

namespace rub
{
	Skybox::Skybox(Device& device, const std::string& environmentPath) : device{ device }
	{
		skyboxModel = std::make_shared<Model>(device, "models/cube.obj");
		cubemap = std::make_unique<Cubemap>(device);

		equiToCube(environmentPath);
		
		std::shared_ptr<Texture> skyboxTexture = std::make_shared<Texture>(device, cubemap->getCaptureImageView());
		std::shared_ptr<Material> skyboxMaterial = std::make_shared<Material>(device, "shaders/skybox.vert.spv", "shaders/skybox.frag.spv");
		skyboxMaterial->addTexture(skyboxTexture);
		skyboxMaterial->setDepthCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL);
		skyboxMaterial->setCullMode(VK_CULL_MODE_FRONT_BIT);

		skyboxObject.model = skyboxModel;
		skyboxObject.transform = { glm::vec3(0), glm::vec3(0) };
		skyboxObject.material = skyboxMaterial;
	}

	void Skybox::equiToCube(const std::string& environmentPath)
	{
		std::shared_ptr<Texture> equiTexture = std::make_shared<Texture>(device, environmentPath, Texture::Format::HDR);
		std::shared_ptr<Material> equiMaterial = std::make_shared<Material>(device, "shaders/equi_to_cube.vert.spv", "shaders/equi_to_cube.frag.spv");
		equiMaterial->addTexture(equiTexture);

		RenderObject equiObject{};
		equiObject.model = skyboxModel;
		equiObject.transform = { glm::vec3(0), glm::vec3(0) };
		equiObject.material = equiMaterial;

		std::vector<RenderObject> renderObjects = { equiObject };

		cubemap->capture(renderObjects);
	}

	void Skybox::draw(VkCommandBuffer commandBuffer)
	{
		skyboxObject.material->bind(commandBuffer);
		skyboxObject.model->bind(commandBuffer);
		skyboxObject.model->draw(commandBuffer, 0);
	}

	Skybox::~Skybox()
	{

	}
}