#pragma once

#include <utils/type.h>

namespace vulkan
{
class command_buffer;
}

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
	void draw(vulkan::command_buffer &command_buffer);

private:
	void generate_docking();
	void generate_console();
	void generate_scene();
	void generate_settings();
	void generate_debug();
	void generate_viewport();

	editor *m_editor = nullptr;
};
