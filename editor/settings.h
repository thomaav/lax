#pragma once

// clang-format off
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop
// clang-format on

struct settings
{
	bool enable_mipmapping;
	bool enable_skybox;
	bool enable_grid;
	VkSampleCountFlagBits sample_count;
	VkFormat color_format;
	VkFormat depth_format;
};
