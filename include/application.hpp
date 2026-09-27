#pragma once

#include "game_object.hpp"
#include "sim_engine_device.hpp"
#include "sim_renderer.hpp"
#include "sim_window.hpp"

// std
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

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
  void loadGameObjects();

  SimWindow simWindow{WIDTH, HEIGHT, "Hello Vulkan!!!"};
  SimEngineDevice simEngineDevice{simWindow};
  Renderer simRenderer{simWindow, simEngineDevice};

  std::vector<SimGameObject> gameObjects;
};
} // namespace Sim
