cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

project(rendering)

# C++ version.
set(CMAKE_CXX_STANDARD 17)

# Dump compile-commands.json.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# Set up directories.
set(CMAKE_BINARY_DIR ./bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_DIR ./lib)
set(LIBRARY_OUTPUT_PATH ${CMAKE_LIBRARY_DIR})

# Include paths for renderer and backends.
include_directories(renderer)
include_directories(renderer/vulkan)

# Build rendering libraries.
file(GLOB RENDERER_SOURCES "${PROJECT_SOURCE_DIR}/renderer/*.cpp")
add_library(renderer STATIC ${RENDERER_SOURCES})

file(GLOB VULKAN_BACKEND_SOURCES "${PROJECT_SOURCE_DIR}/renderer/vulkan/*.cpp")
add_library(vulkan_backend STATIC ${VULKAN_BACKEND_SOURCES})

# Include applications.
include_directories(basic)

# Build applications.
add_executable(basic ${PROJECT_SOURCE_DIR}/basic/main.cpp)

# Add library dependencies to applications.
target_link_libraries(basic renderer)
target_link_libraries(basic vulkan_backend)

# Add external dependencies to applications.
find_package(glfw3 3.3 REQUIRED FATAL_ERROR)
target_link_libraries(basic glfw)

find_package(Vulkan REQUIRED FATAL_ERROR)
target_link_libraries(basic vulkan)
