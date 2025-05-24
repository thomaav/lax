#pragma once

#include <renderer/vulkan/command_buffer.h>

#include "log.h"
#include "scene.h"
#include "settings.h"
#include "ui.h"

class editor
{
public:
	editor() = default;
	~editor() = default;

	editor(const editor &) = delete;
	editor operator=(const editor &) = delete;

	void build();

	void update();
	void draw(vulkan::command_buffer &command_buffer);

	vulkan::context m_context = {};
	settings m_settings = {};
	scene m_scene = {};
	ui m_ui = {};
	console_logger m_logger = {};

private:
	void build_default_settings();
};
