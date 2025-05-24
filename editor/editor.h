#pragma once

#include <renderer/vulkan/command_buffer.h>

#include "log.h"
#include "scene.h"
#include "ui.h"

struct settings
{
	/* Configurable at runtime. */
	bool enable_mipmapping;
	bool enable_skybox;
	bool enable_grid;
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

	void build_default();
	void draw(vulkan::command_buffer &command_buffer);

	vulkan::context m_context = {};
	settings m_settings = {};
	scene m_scene = {};
	ui m_ui = {};
	console_logger m_logger = {};

private:
	void set_default_settings();
};
