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
  VkExtent2D getExtent() {
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
  }
  bool wasWindowResized() { return framebufferResized; }
  void resetWindowResizedFlag() { framebufferResized = false; }

  void createWindowSurface(VkInstance instace, VkSurfaceKHR *surface);

private:
  static void framebufferResizedCallback(GLFWwindow *window, int width,
                                         int height);
  void initWindow();

  int width;
  int height;
  bool framebufferResized = false;

  std::string windowName;

  GLFWwindow *window;
};
} // namespace Sim
