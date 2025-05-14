#pragma once

#include <renderer/scene.h>

struct settings
{
	/* Configurable at runtime. */
	bool enable_mipmapping;
	bool enable_skybox;
	VkSampleCountFlagBits sample_count;

	/* Static. */
	VkFormat color_format;
	VkFormat depth_format;
};

class editor
{
public:
	editor() = default;
	~editor() = default;

	editor(const editor &) = delete;
	editor operator=(const editor &) = delete;

	void build_default(vulkan::context &context);

	settings m_settings = {};
	scene m_scene = {};

private:
};
