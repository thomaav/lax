#pragma once

#include <renderer/vulkan/context.h>

class editor;

class ui
{
public:
	ui() = default;
	~ui();

	ui(const ui &) = delete;
	ui operator=(const ui &) = delete;

	void build(editor &editor);
	void generate_frame();
	void draw();

	u32 m_viewport_x;
	u32 m_viewport_y;
	u32 m_viewport_width;
	u32 m_viewport_height;

private:
	void generate_docking();
	void generate_console();
	void generate_scene();
	void generate_settings();
	void generate_debug();
	void generate_viewport();

	editor *m_editor = nullptr;
};
