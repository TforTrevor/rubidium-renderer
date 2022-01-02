#pragma once

#include "device.hpp"

#include <string>
#include <vector>

namespace rub
{
	struct PipelineConfigInfo
	{
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;

		VkDescriptorSetLayout descriptorSetLayout = nullptr;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class Pipeline
	{
	public:
		Pipeline(Device& device, const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
		Pipeline(Device& device, const std::string& compPath, const VkPipelineLayout pipelineLayout);
		~Pipeline();

		void bind(VkCommandBuffer commandBuffer);

		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

	private:
		Device& device;
		VkPipeline pipeline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
		VkShaderModule compShaderModule;
		const bool isCompute = false;

		static std::vector<char> readFile(const std::string& filePath);

		void createGraphicsPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
		void createComputePipeline(const std::string& compPath, const VkPipelineLayout pipelineLayout);
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
	};
}