#pragma once

#include <cassert>
#include <iostream>
#include <stdio.h>

#include <utils/util.h>

// clang-format off
#define VULKAN_ASSERT_SUCCESS(statement) do { \
	VkResult res = statement;                 \
	UNUSED(res);                              \
	assert(res == VK_SUCCESS);                \
} while (0)
#define VULKAN_ASSERT_NOT_NULL(handle) assert(handle != VK_NULL_HANDLE)
// clang-format on
