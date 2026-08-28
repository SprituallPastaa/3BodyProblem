#pragma once

#include "sim_engine_device.hpp"
#include "sim_pipeline.hpp"
#include "sim_window.hpp"
#include "my_engine_swap_chain.hpp"

// std
#include <memory>
#include <vector>

namespace Sim {
class FirstApp {
public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  FirstApp();
  ~FirstApp();

  FirstApp(const FirstApp &) = delete;
  FirstApp &operator=(const FirstApp &) = delete;

  void run();

private:
void createPipelineLayout();
void createPipeline();
void createCommandBuffers();
void drawFrame();

  SimWindow simWindow{WIDTH, HEIGHT, "Hello Vulkan!!!"};
  SimEngineDevice simEngineDevice{simWindow};
  MyEngineSwapChain simSwapChain{simEngineDevice, simWindow.getExtent()};
  std::unique_ptr<SimPipeline> simPipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<VkCommandBuffer> commandBuffers;
};
} // namespace Sim
