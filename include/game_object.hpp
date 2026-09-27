#pragma once

#include "sim_model.hpp"

// std
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_float2x2.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/trigonometric.hpp>
#include <memory>

namespace Sim {
struct Transform2DComponent {
  glm::vec2 translation{}; // (position offset)
  glm::vec2 scale{1.f, 1.f};
  float rotation;

  glm::mat2 mat2() {
    const float s = glm::sin(rotation);
    const float c = glm::cos(rotation);

    glm::mat2 rotationMat{{c, s}, {-s, c}};
    glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
    return rotationMat * scaleMat;
  }
};

class SimGameObject {
public:
  using id_t = unsigned int;

  static SimGameObject createGameObject() {
    static id_t currentId = 0;
    return SimGameObject(currentId++);
  }

  SimGameObject(const SimGameObject &) = delete;
  SimGameObject &operator=(const SimGameObject &) = delete;
  SimGameObject(SimGameObject &&) = default;
  SimGameObject &operator=(SimGameObject &&) = default;

  const id_t getId() { return id; }

  std::shared_ptr<SimModel> model{};
  glm::vec3 color{};
  Transform2DComponent transform2D{};

private:
  SimGameObject(id_t objId) : id{objId} {}

  id_t id;
};

} // namespace Sim
