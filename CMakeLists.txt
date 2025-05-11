cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# Compiler.
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

# C++ version.
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall -Wextra)

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
add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/volk" volk)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/assimp" assimp)
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

# (TODO, thoave01): Remove assets from backend.
file(GLOB VULKAN_BACKEND_SOURCES "${PROJECT_SOURCE_DIR}/renderer/vulkan/*.cpp" "${PROJECT_SOURCE_DIR}/assets/*.cpp")
add_library(vulkan_backend STATIC ${VULKAN_BACKEND_SOURCES})
target_link_libraries(vulkan_backend PRIVATE
  volk
  assimp
  spirv-cross-core
  GPUOpen::VulkanMemoryAllocator
  imgui
)

file(GLOB PLATFORM_SOURCES "${PROJECT_SOURCE_DIR}/platform/*.cpp")
add_library(platform STATIC ${PLATFORM_SOURCES})
target_link_libraries(platform PRIVATE volk)

if(APPLE)
  file(GLOB METAL_BACKEND_CPP_SOURCES "${PROJECT_SOURCE_DIR}/renderer/metal/*.cpp" "${PROJECT_SOURCE_DIR}/model/*.cpp") # (TODO, thoave01): Remove model.
  file(GLOB METAL_BACKEND_MM_SOURCES "${PROJECT_SOURCE_DIR}/renderer/metal/*.mm")
  set_source_files_properties(${METAL_BACKEND_MM_SOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++")
  add_library(metal_backend STATIC ${METAL_BACKEND_CPP_SOURCES} ${METAL_BACKEND_MM_SOURCES})
  target_link_libraries(metal_backend PRIVATE
	"-framework Metal"
    "-framework MetalKit"
    "-framework AppKit"
    "-framework Foundation"
    "-framework QuartzCore"
	volk
	assimp
	glm # (TODO, thoave01): Also use precompiled for Vulkan.
  )
endif()

# Include paths for renderer and backends.
target_include_directories(vulkan_backend PUBLIC ${CMAKE_SOURCE_DIR})
target_include_directories(vulkan_backend PUBLIC ${VULKAN_SDK_INCLUDE_DIR})
target_include_directories(vulkan_backend SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/third_party/assimp/include")
target_include_directories(vulkan_backend SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/third_party/stb")
target_include_directories(vulkan_backend SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/third_party/SPIRV-Cross")
target_include_directories(vulkan_backend SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/third_party/VulkanMemoryAllocator")
target_include_directories(vulkan_backend SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/third_party/") # (TODO, thoave01): imgui as <imgui/>.
if(APPLE)
  target_include_directories(metal_backend PUBLIC ${CMAKE_SOURCE_DIR})
  target_include_directories(metal_backend SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/third_party/metal-cpp-cmake/metal-cmake/metal-cpp" "${CMAKE_SOURCE_DIR}/third_party/metal-cpp-cmake/metal-cmake/metal-cpp-extensions")
  target_include_directories(metal_backend SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/third_party/assimp/include")
  target_include_directories(metal_backend SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/third_party/stb")
endif()
target_include_directories(platform PUBLIC ${CMAKE_SOURCE_DIR})

# Include applications.
include_directories(basic)

# Build applications.
add_executable(basic ${PROJECT_SOURCE_DIR}/basic/main.cpp)

# Add library dependencies to applications.
target_link_libraries(basic vulkan_backend)
if(APPLE)
  target_link_libraries(basic metal_backend)
endif()
target_link_libraries(basic platform)

# Add external dependencies to applications.
target_link_libraries(basic glfw)
target_link_libraries(basic vulkan)

# Not added by default on Mac, but it's where the Vulkan SDK puts the global installation.
if(APPLE)
  target_include_directories(basic PUBLIC /usr/local/include)
  target_link_directories(basic PUBLIC /usr/local/lib)
endif()

# Add shaders to applications.
add_custom_target(copy_shaders
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}/assets/shaders ${BASH_EXECUTABLE} ./compile.sh
  # (TODO, thoave01): This overwrites assets if you're in ${CMAKE_SOURCE_DIR}.
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/bin/assets
)
add_dependencies(basic copy_shaders)
