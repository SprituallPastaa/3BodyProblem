#include "application.hpp"

#include <cstdlib>
#include <print>
#include <stdexcept>

int main() {
  Sim::FirstApp app{};
  try {
    app.run();
  } catch (const std::exception &err) {
    std::println("{}", err.what());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
