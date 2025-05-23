#pragma once

// clang-format off
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#include <third_party/imgui/imgui.h>
#include <third_party/imgui/imgui_impl_glfw.h>
#include <third_party/imgui/imgui_impl_vulkan.h>
#pragma clang diagnostic pop
// clang-format on

#include <renderer/vulkan/command_buffer.h>

#include "log.h"
#include "scene.h"

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
	~editor();

	editor(const editor &) = delete;
	editor operator=(const editor &) = delete;

	void build_default();
	void draw(vulkan::command_buffer &command_buffer);

	vulkan::context m_context = {};
	settings m_settings = {};
	scene m_scene = {};
	console_logger m_logger = {};

private:
};
