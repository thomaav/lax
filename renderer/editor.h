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

#include <renderer/scene.h>
#include <utils/log.h>

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

class console_logger : public loggerp
{
public:
	console_logger() = default;
	~console_logger() = default;

	console_logger(const console_logger &) = delete;
	console_logger operator=(const console_logger &) = delete;

	void log(const char *str) override;

	ImGuiTextBuffer m_buffer = {};
};

class editor
{
public:
	editor() = default;
	~editor();

	editor(const editor &) = delete;
	editor operator=(const editor &) = delete;

	void build_default(vulkan::context &context);
	void draw(vulkan::command_buffer &command_buffer);

	settings m_settings = {};
	scene m_scene = {};
	console_logger m_logger = {};

private:
};
