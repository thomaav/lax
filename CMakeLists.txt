cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

project(rendering)

set(CMAKE_BINARY_DIR ./bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_DIR ./lib)
set(LIBRARY_OUTPUT_PATH ${CMAKE_LIBRARY_DIR})

# Include renderer.
include_directories(renderer)
include_directories(renderer/vulkan)

# Include applications.
include_directories(basic)

# Build rendering libraries.

# Build applications.
add_executable(basic ${PROJECT_SOURCE_DIR}/basic/main.cpp)
