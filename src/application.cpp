#include "application.hpp"
#include <GLFW/glfw3.h>

namespace Sim {

void FirstApp::run() {
  while (!simWindow.shouldClose()) {
    glfwPollEvents();
  }
}
} // namespace Sim
