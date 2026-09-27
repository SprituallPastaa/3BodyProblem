#include "sim_renderer.hpp"
#include "game_object.hpp"
#include "my_engine_swap_chain.hpp"
#include "sim_engine_device.hpp"
#include "sim_model.hpp"
#include "sim_pipeline.hpp"
#include <glm/common.hpp>

// std
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <glm/detail/qualifier.hpp>
#include <memory>
#include <print>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Sim {

Renderer::Renderer(SimWindow &window, SimEngineDevice &device)
    : simWindow{window}, simEngineDevice{device} {
  recreateSwapChain();
  createCommandBuffers();
}

Renderer::~Renderer() { freeCommandBuffers(); }

void Renderer::recreateSwapChain() {
  auto extent = simWindow.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = simWindow.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(simEngineDevice.device());
  if (simSwapChain == nullptr) {
    simSwapChain = std::make_unique<MyEngineSwapChain>(simEngineDevice, extent);
  } else {
    std::shared_ptr<MyEngineSwapChain> oldSwapChain = std::move(simSwapChain);
    simSwapChain = std::make_unique<MyEngineSwapChain>(simEngineDevice, extent,
                                                       oldSwapChain);

    if (!oldSwapChain->compareSwapFormats(*simSwapChain.get())) {
      throw std::runtime_error("Swap Chain image format has changed");
    }
  }
  // TODO i just removed the create pipeline function, put something else here
  // instead
}

void Renderer::createCommandBuffers() {
  commandBuffers.resize(MyEngineSwapChain::MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = simEngineDevice.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(simEngineDevice.device(), &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers");
  }
}

void Renderer::freeCommandBuffers() {
  vkFreeCommandBuffers(
      simEngineDevice.device(), simEngineDevice.getCommandPool(),
      static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
  commandBuffers.clear();
}

VkCommandBuffer Renderer::beginFrame() {
  assert(!isFrameStarted &&
         "Can't call beginFrame with a frame already in progress");

  auto result = simSwapChain->acquireNextImage(&currentImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to aquire swap chain image");
  }

  isFrameStarted = true;

  auto commandBuffer = getCurrentCommandBuffer();

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("Command buffer failed to begin recording");
  }
  return commandBuffer;
}

void Renderer::endFrame() {
  assert(isFrameStarted &&
         "Can't call endFrame while frame is not in progress");
  auto commandBuffer = getCurrentCommandBuffer();

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to record command buffer");
  }

  auto result =
      simSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      simWindow.wasWindowResized()) {
    simWindow.resetWindowResizedFlag();
    recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image");
  }

  isFrameStarted = false;
  currentFrameIndex =
      (currentFrameIndex + 1) % MyEngineSwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted &&
         "Can't call beginSwapChainRenderPass while frame is not in progress");
  assert(commandBuffer == getCurrentCommandBuffer() &&
         "Can't begin render pass on commandbuffer from a different frame");

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

  renderPassInfo.renderPass = simSwapChain->getRenderPass();
  renderPassInfo.framebuffer = simSwapChain->getFrameBuffer(currentImageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = simSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.height =
      static_cast<float>(simSwapChain->getSwapChainExtent().height);
  viewport.width = static_cast<float>(simSwapChain->getSwapChainExtent().width);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, simSwapChain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted &&
         "Can't call endSwapChainRenderPass is not in progress");
  assert(commandBuffer == getCurrentCommandBuffer() &&
         "Can't end render pass on commandbuffer from a different frame");

  vkCmdEndRenderPass(commandBuffer);
}
} // namespace Sim
