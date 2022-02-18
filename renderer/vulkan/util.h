#pragma once

#include <cassert>

#define VULKAN_ASSERT_SUCCESS(statement) assert(statement == VK_SUCCESS)
#define VULKAN_ASSERT_NOT_NULL(handle) assert(handle != VK_NULL_HANDLE)
