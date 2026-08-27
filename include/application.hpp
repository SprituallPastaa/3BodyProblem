#pragma once

#include "sim_engine_device.hpp"
#include "sim_pipeline.hpp"
#include "sim_window.hpp"

namespace Sim {
class FirstApp {
public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  void run();

private:
  SimWindow simWindow{WIDTH, HEIGHT, "Hello Vulkan!!!"};
  SimEngineDevice simEngineDevice{simWindow};
  SimPipeline simPipeline{
      simEngineDevice, "shaders/simple_shader.vert.spv",
      "shaders/simple_shader.frag.spv",
      SimPipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
};
} // namespace Sim
