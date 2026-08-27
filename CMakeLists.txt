cmake_minimum_required(VERSION 3.12)

project(physics_sim LANGUAGES CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)

add_executable(pastasim
  ${SOURCES}
)

target_include_directories(pastasim
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(pastasim PRIVATE
  Vulkan::Vulkan
  glfw
  glm::glm
)


