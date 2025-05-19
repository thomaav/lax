#pragma once

#include <renderer/log.h>
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

	/* (TODO): */
	/* - resolution, which needs swapchain recreation */
};

class editor
{
public:
	editor() = default;
	~editor() = default;

	editor(const editor &) = delete;
	editor operator=(const editor &) = delete;

	void build_default(vulkan::context &context);
	void draw(vulkan::command_buffer &command_buffer);

	settings m_settings = {};
	scene m_scene = {};
	console_logger m_logger = {};

private:
};
