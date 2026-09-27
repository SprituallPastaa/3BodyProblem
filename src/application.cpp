#include "application.hpp"
#include "render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <GLFW/glfw3.h>
#include <glm/ext/vector_float2.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace Sim {
FirstApp::FirstApp() { loadGameObjects(); }

FirstApp::~FirstApp() {}

void FirstApp::run() {
  SimpleRenderSystem renderSystem{simEngineDevice,
                                  simRenderer.getSwapChainRenderPass()};

  while (!simWindow.shouldClose()) {
    glfwPollEvents();

    if (auto commandBuffer = simRenderer.beginFrame()) {

      simRenderer.beginSwapChainRenderPass(commandBuffer);
      renderSystem.renderGameObjects(commandBuffer, gameObjects);
      simRenderer.endSwapChainRenderPass(commandBuffer);
      simRenderer.endFrame();
    }

    // vkDeviceWaitIdle(simEngineDevice.device());
    // std::println("{}",
    // simEngineDevice.properties.limits.maxPushConstantsSize);
  }
}

void FirstApp::loadGameObjects() {
  std::vector<SimModel::Vertex> colorVertices{
      {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
  auto simModel = std::make_shared<SimModel>(simEngineDevice, colorVertices);

  auto triangle = SimGameObject::createGameObject();
  triangle.model = simModel;
  triangle.color = {.1f, .8f, .1f};
  triangle.transform2D.translation.x = .2f;
  triangle.transform2D.scale = {2.f, 0.5f};
  triangle.transform2D.rotation = .25f * glm::two_pi<float>();

  gameObjects.push_back(std::move(triangle));
}

} // namespace Sim
