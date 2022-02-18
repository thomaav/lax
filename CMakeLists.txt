cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

# C++ version.
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall -Wextra -pedantic -Werror)

# Dump compile-commands.json.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# Create the project.
project(rendering)

# Set up directories.
set(CMAKE_BINARY_DIR ./bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_DIR ./lib)
set(LIBRARY_OUTPUT_PATH ${CMAKE_LIBRARY_DIR})

# Build libraries.
# file(GLOB RENDERER_SOURCES "${PROJECT_SOURCE_DIR}/renderer/*.cpp")
# add_library(renderer STATIC ${RENDERER_SOURCES})

file(GLOB VULKAN_BACKEND_SOURCES "${PROJECT_SOURCE_DIR}/renderer/vulkan/*.cpp")
add_library(vulkan_backend STATIC ${VULKAN_BACKEND_SOURCES})

file(GLOB PLATFORM_SOURCES "${PROJECT_SOURCE_DIR}/platform/*.cpp")
add_library(platform STATIC ${PLATFORM_SOURCES})

# Include paths for renderer and backends.
# target_include_directories(renderer PUBLIC ${CMAKE_SOURCE_DIR})
target_include_directories(vulkan_backend PUBLIC ${CMAKE_SOURCE_DIR})
target_include_directories(platform PUBLIC ${CMAKE_SOURCE_DIR})

# Include applications.
include_directories(basic)

# Build applications.
add_executable(basic ${PROJECT_SOURCE_DIR}/basic/main.cpp)

# Add library dependencies to applications.
# target_link_libraries(basic renderer)
target_link_libraries(basic vulkan_backend)
target_link_libraries(basic platform)

# Add external dependencies to applications.
find_package(glfw3 3.3 REQUIRED FATAL_ERROR)
target_link_libraries(basic glfw)

find_package(Vulkan REQUIRED FATAL_ERROR)
target_link_libraries(basic vulkan)
