#include "sim_window.hpp"
#include <stdexcept>

namespace Sim {

SimWindow::SimWindow(int w, int h, std::string name)
    : width{w}, height(h), windowName{name} {
  initWindow();
}

SimWindow::~SimWindow() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void SimWindow::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window =
      glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
}

void SimWindow::createWindowSurface(VkInstance instace, VkSurfaceKHR *surface) {
  if (glfwCreateWindowSurface(instace, window, nullptr, surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface");
  }
}
} // namespace Sim
