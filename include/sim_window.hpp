#pragma once

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace Sim {

class SimWindow {
public:
  SimWindow(int w, int h, std::string name);
  ~SimWindow();

  SimWindow(const SimWindow &) = delete;
  SimWindow &operator=(const SimWindow &) = delete;

  bool shouldClose() { return glfwWindowShouldClose(window); }
  VkExtent2D getExtent() { return {static_cast<uint32_t>(width),static_cast<uint32_t>(height)};}

  void createWindowSurface(VkInstance instace, VkSurfaceKHR *surface);

private:
  void initWindow();

  const int width;
  const int height;

  std::string windowName;

  GLFWwindow *window;
};
} // namespace Sim
