cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall -Wextra)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

project(lax)
set(CMAKE_BINARY_DIR ./bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_DIR ./lib)
set(LIBRARY_OUTPUT_PATH ${CMAKE_LIBRARY_DIR})

add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/volk" volk)
add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/SPIRV-Cross" spirv_cross)
set(VMA_BUILD_SAMPLE OFF)
set(VMA_IMPLEMENTATION_BUILD ON)
add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/VulkanMemoryAllocator" vma)
set(imgui_SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui/)
add_library(imgui
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/imgui_impl_vulkan.cpp
)
target_compile_definitions(imgui PRIVATE IMGUI_IMPL_VULKAN_USE_VOLK=1)
target_include_directories(imgui PRIVATE "${CMAKE_SOURCE_DIR}/third_party/volk")

file(GLOB EDITOR_SOURCES "${PROJECT_SOURCE_DIR}/editor/*.cpp")
file(GLOB ASSETS_SOURCES "${PROJECT_SOURCE_DIR}/assets/*.cpp")
file(GLOB RENDERER_SOURCES "${PROJECT_SOURCE_DIR}/renderer/*.cpp")
file(GLOB UTILS_SOURCES "${PROJECT_SOURCE_DIR}/utils/*.cpp")
file(GLOB PLATFORM_SOURCES "${PROJECT_SOURCE_DIR}/platform/*.cpp")
file(GLOB VULKAN_BACKEND_SOURCES "${PROJECT_SOURCE_DIR}/renderer/vulkan/*.cpp")

add_executable(editor
  ${EDITOR_SOURCES}
  ${ASSETS_SOURCES}
  ${RENDERER_SOURCES}
  ${UTILS_SOURCES}
  ${PLATFORM_SOURCES}
  ${VULKAN_BACKEND_SOURCES}
)
target_link_libraries(editor
  glfw
  volk
  assimp
  spirv-cross-core
  GPUOpen::VulkanMemoryAllocator
  imgui
)
target_include_directories(editor
  PUBLIC ${CMAKE_SOURCE_DIR}
  PUBLIC ${VULKAN_SDK_INCLUDE_DIR}
  PUBLIC "${CMAKE_SOURCE_DIR}/third_party/"
  PUBLIC "${CMAKE_SOURCE_DIR}/third_party/stb"
  PUBLIC "${CMAKE_SOURCE_DIR}/third_party/SPIRV-Cross"
)
add_custom_target(copy_shaders
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}/assets/shaders ${BASH_EXECUTABLE} ./compile.sh
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/bin/assets
)
add_dependencies(editor copy_shaders)
