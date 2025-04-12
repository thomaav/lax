cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# C++ version.
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall -Wextra -pedantic -Werror)

# Dump compile-commands.json.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# Create the project.
project(lax)

# Set up directories.
set(CMAKE_BINARY_DIR ./bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_DIR ./lib)
set(LIBRARY_OUTPUT_PATH ${CMAKE_LIBRARY_DIR})

# Build libraries.
file(GLOB VULKAN_BACKEND_SOURCES "${PROJECT_SOURCE_DIR}/renderer/vulkan/*.cpp")
add_library(vulkan_backend STATIC ${VULKAN_BACKEND_SOURCES})

add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/volk" volk)
target_link_libraries(vulkan_backend PRIVATE volk)

file(GLOB PLATFORM_SOURCES "${PROJECT_SOURCE_DIR}/platform/*.cpp")
add_library(platform STATIC ${PLATFORM_SOURCES})

# Include paths for renderer and backends.
target_include_directories(vulkan_backend PUBLIC ${CMAKE_SOURCE_DIR})
target_include_directories(platform PUBLIC ${CMAKE_SOURCE_DIR})

# Include applications.
include_directories(basic)

# Build applications.
add_executable(basic ${PROJECT_SOURCE_DIR}/basic/main.cpp)

# Add library dependencies to applications.
target_link_libraries(basic vulkan_backend)
target_link_libraries(basic platform)

# Add external dependencies to applications.
target_link_libraries(basic glfw)
target_link_libraries(basic vulkan)

# Add shaders to applications.
add_custom_target(copy_shaders
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}/basic/shaders ${BASH_EXECUTABLE} ./compile.sh
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/basic/shaders ${CMAKE_CURRENT_BINARY_DIR}/bin/shaders
)
add_dependencies(basic copy_shaders)
