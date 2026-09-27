#pragma once

#include "my_engine_swap_chain.hpp"
#include "sim_engine_device.hpp"
#include "sim_window.hpp"

// std
#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Sim {
class Renderer {
public:
  Renderer(SimWindow &window, SimEngineDevice &device);
  ~Renderer();

  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  VkRenderPass getSwapChainRenderPass() const {
    return simSwapChain->getRenderPass();
  }

  bool isFrameInProgress() const { return isFrameStarted; };

  VkCommandBuffer getCurrentCommandBuffer() const {
    assert(isFrameStarted &&
           "Cannot get command buffer when frame not in progress");
    return commandBuffers[currentFrameIndex];
  };

  int getFrameIndex() const {
    assert(isFrameStarted &&
           "Cannot get frame index when frame not in progress");
    return currentFrameIndex;
  }

  VkCommandBuffer beginFrame();
  void endFrame();
  void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();

  SimWindow &simWindow;
  SimEngineDevice &simEngineDevice;
  std::unique_ptr<MyEngineSwapChain> simSwapChain;
  std::vector<VkCommandBuffer> commandBuffers;

  uint32_t currentImageIndex;
  int currentFrameIndex = 0;
  bool isFrameStarted = false;
};
} // namespace Sim
