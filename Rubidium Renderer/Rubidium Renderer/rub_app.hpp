#pragma once

#include "rub_window.hpp"
#include "rub_pipeline.hpp"
#include "rub_device.hpp"
#include "rub_swap_chain.hpp"

#include <memory>
#include <vector>

namespace rub
{
	class RubApp
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		RubApp();
		~RubApp();

		void run();

	private:
		RubWindow rubWindow{ WIDTH, HEIGHT, "Rubidium Renderer" };
		RubDevice rubDevice{ rubWindow };
		RubSwapChain rubSwapChain{ rubDevice, rubWindow.getExtent() };
		std::unique_ptr<RubPipeline> rubPipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;

		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void drawFrame();
	};
}