#pragma once

#include "my_engine_swap_chain.hpp"
#include "sim_engine_device.hpp"
#include "sim_model.hpp"
#include "sim_pipeline.hpp"
#include "sim_window.hpp"

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
  void loadModels();
  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void freeCommandBuffers();
  void drawFrame();
  void recreateSwapChain();
  void recordCommandBuffer(int imageIndex);

  SimWindow simWindow{WIDTH, HEIGHT, "Hello Vulkan!!!"};
  SimEngineDevice simEngineDevice{simWindow};
  std::unique_ptr<MyEngineSwapChain> simSwapChain;
  std::unique_ptr<SimPipeline> simPipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<VkCommandBuffer> commandBuffers;
  std::unique_ptr<SimModel> simModel;
};
} // namespace Sim
