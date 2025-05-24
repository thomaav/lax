#pragma once

// clang-format off
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop
// clang-format on

#include <utils/type.h>

struct settings
{
	/* Can be set by editor. */
	bool enable_mipmapping;
	bool enable_skybox;
	bool enable_grid;
	VkSampleCountFlagBits sample_count;
	VkFormat color_format;
	VkFormat depth_format;

	/* Should not be set by the editor. */
	u32 viewport_x;
	u32 viewport_y;
	u32 viewport_width;
	u32 viewport_height;
};
