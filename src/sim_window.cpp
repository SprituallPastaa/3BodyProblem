#include "sim_window.hpp"
#include <GLFW/glfw3.h>
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
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window =
      glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizedCallback);
}

void SimWindow::createWindowSurface(VkInstance instace, VkSurfaceKHR *surface) {
  if (glfwCreateWindowSurface(instace, window, nullptr, surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface");
  }
}

void SimWindow::framebufferResizedCallback(GLFWwindow *window, int width,
                                           int height) {
  auto simWindow =
      reinterpret_cast<SimWindow *>(glfwGetWindowUserPointer(window));
  simWindow->framebufferResized = true;
  simWindow->width = width;
  simWindow->height = height;
}

} // namespace Sim
