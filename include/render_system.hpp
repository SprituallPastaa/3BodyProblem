#pragma once

#include "game_object.hpp"
#include "sim_engine_device.hpp"
#include "sim_pipeline.hpp"

// std
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Sim {
class SimpleRenderSystem {
public:
  SimpleRenderSystem(SimEngineDevice &device, VkRenderPass renderPass);
  ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

  void renderGameObjects(VkCommandBuffer commandBuffer,
                         std::vector<SimGameObject> &gameObjects);

private:
  void createPipelineLayout();
  void createPipeline(VkRenderPass renderPass);

  SimEngineDevice &simDevice;

  std::unique_ptr<SimPipeline> simPipeline;
  VkPipelineLayout pipelineLayout;
};
} // namespace Sim
