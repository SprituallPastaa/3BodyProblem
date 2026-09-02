#include "application.hpp"
#include "game_object.hpp"
#include "my_engine_swap_chain.hpp"
#include "sim_model.hpp"
#include "sim_pipeline.hpp"
#include <glm/common.hpp>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <GLFW/glfw3.h>
#include <glm/ext/vector_float2.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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

struct SimplePushConstantData {
  glm::mat2 transform{1.f};
  glm::vec2 offset;
  alignas(16) glm::vec3 color;
};

FirstApp::FirstApp() {
  loadGameObjects();
  createPipelineLayout();
  recreateSwapChain();
  createCommandBuffers();
}

FirstApp::~FirstApp() {
  vkDestroyPipelineLayout(simEngineDevice.device(), pipelineLayout, nullptr);
}

void FirstApp::run() {
  while (!simWindow.shouldClose()) {
    glfwPollEvents();
    drawFrame();
    vkDeviceWaitIdle(simEngineDevice.device());
    // std::println("{}",
    // simEngineDevice.properties.limits.maxPushConstantsSize);
  }
}

void sierpinsky(const std::vector<SimModel::Vertex> &inVertices, int depth,
                std::vector<SimModel::Vertex> &outVertices) {
  if (depth == 0) {
    outVertices.push_back(inVertices[0]);
    outVertices.push_back(inVertices[1]);
    outVertices.push_back(inVertices[2]);
    return;
  }

  // Helper lambda to calculate midpoint including all axes
  auto getMidpoint = [](const SimModel::Vertex &v1,
                        const SimModel::Vertex &v2) {
    SimModel::Vertex mid = v1;
    mid.position.x = (v1.position.x + v2.position.x) / 2.0f;
    mid.position.y = (v1.position.y + v2.position.y) / 2.0f;
    return mid;
  };

  SimModel::Vertex m0 = getMidpoint(inVertices[0], inVertices[1]);
  SimModel::Vertex m1 = getMidpoint(inVertices[1], inVertices[2]);
  SimModel::Vertex m2 = getMidpoint(inVertices[2], inVertices[0]);

  // 3 recursive calls for the 3 corner sub-triangles
  sierpinsky({inVertices[0], m0, m2}, depth - 1, outVertices); // Sub-triangle 1
  sierpinsky({m0, inVertices[1], m1}, depth - 1, outVertices); // Sub-triangle 2
  sierpinsky({m2, m1, inVertices[2]}, depth - 1, outVertices); // Sub-triangle 3
}

void FirstApp::loadGameObjects() {
  std::vector<SimModel::Vertex> inVertices{
      {{0.0f, -0.5f}}, {{0.5f, 0.5f}}, {{-0.5f, 0.5f}}};
  std::vector<SimModel::Vertex> colorVertices{
      {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
  std::vector<SimModel::Vertex> outVertices;
  sierpinsky(inVertices, 5, outVertices);
  auto simModel = std::make_shared<SimModel>(simEngineDevice, colorVertices);

  auto triangle = SimGameObject::createGameObject();
  triangle.model = simModel;
  triangle.color = {.1f, .8f, .1f};
  triangle.transform2D.translation.x = .2f;
  triangle.transform2D.scale = {2.f, 0.5f};
  triangle.transform2D.rotation = .25f * glm::two_pi<float>();

  gameObjects.push_back(std::move(triangle));
}

void FirstApp::createPipelineLayout() {

  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantData);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(simEngineDevice.device(), &pipelineLayoutInfo,
                             nullptr, &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout");
  }
}

void FirstApp::createPipeline() {
  assert(simSwapChain != nullptr && "Cannot create pipeline before swap chain");
  assert(pipelineLayout != nullptr &&
         "Cannot create pipeline before pieline layout");

  PipelineConfigInfo pipelineConfig{};
  SimPipeline::defaultPipelineConfigInfo(pipelineConfig);

  pipelineConfig.renderPass = simSwapChain->getRenderPass();
  pipelineConfig.pipelineLayout = pipelineLayout;

  simPipeline = std::make_unique<SimPipeline>(
      simEngineDevice, "shaders/simple_shader.vert.spv",
      "shaders/simple_shader.frag.spv", pipelineConfig);
}

void FirstApp::recreateSwapChain() {
  auto extent = simWindow.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = simWindow.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(simEngineDevice.device());
  if (simSwapChain == nullptr) {
    simSwapChain = std::make_unique<MyEngineSwapChain>(simEngineDevice, extent);
  } else {
    simSwapChain = std::make_unique<MyEngineSwapChain>(simEngineDevice, extent,
                                                       std::move(simSwapChain));
    if (simSwapChain->imageCount() != commandBuffers.size()) {
      freeCommandBuffers();
      createCommandBuffers();
    }
  }
  createPipeline();
}

void FirstApp::createCommandBuffers() {
  commandBuffers.resize(simSwapChain->imageCount());

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

void FirstApp::freeCommandBuffers() {
  vkFreeCommandBuffers(
      simEngineDevice.device(), simEngineDevice.getCommandPool(),
      static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
  commandBuffers.clear();
}

void FirstApp::recordCommandBuffer(int imageIndex) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) !=
      VK_SUCCESS) {
    throw std::runtime_error("Command buffer failed to begin recording");
  }

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

  renderPassInfo.renderPass = simSwapChain->getRenderPass();
  renderPassInfo.framebuffer = simSwapChain->getFrameBuffer(imageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = simSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo,
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
  vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
  vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

  renderGameObjects(commandBuffers[imageIndex]);
  vkCmdEndRenderPass(commandBuffers[imageIndex]);

  if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
    throw std::runtime_error("Failed to record command buffer");
  }
}

void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer) {
  simPipeline->bind(commandBuffer);

  for (auto &obj : gameObjects) {
    obj.transform2D.rotation =
        glm::mod(obj.transform2D.rotation + .01f, glm::two_pi<float>());

    SimplePushConstantData push{};
    push.offset = {obj.transform2D.translation};
    push.color = {obj.color};
    push.transform = {obj.transform2D.mat2()};

    vkCmdPushConstants(commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_FRAGMENT_BIT |
                           VK_SHADER_STAGE_VERTEX_BIT,
                       0, sizeof(SimplePushConstantData), &push);
    obj.model->bind(commandBuffer);
    obj.model->draw(commandBuffer);
  }
}

void FirstApp::drawFrame() {
  uint32_t imageIndex;
  auto result = simSwapChain->acquireNextImage(&imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to aquire swap chain image");
  }

  recordCommandBuffer(imageIndex);
  result = simSwapChain->submitCommandBuffers(&commandBuffers[imageIndex],
                                              &imageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      simWindow.wasWindowResized()) {
    simWindow.resetWindowResizedFlag();
    recreateSwapChain();
    return;
  }
  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image");
  }
}
} // namespace Sim
