#include <algorithm>

#include <renderer/vulkan/buffer.h>
#include <renderer/vulkan/command_buffer.h>
#include <renderer/vulkan/context.h>
#include <renderer/vulkan/image.h>

#include "render_graph.h"

render_pass::render_pass(render_graph &rg)
    : m_render_graph(rg)
{
}

void render_pass::execute(vulkan::command_buffer &cmd_buf)
{
	m_execution_function(cmd_buf);
}

render_buffer &render_pass::add_buffer(const std::string_view &name)
{
	return m_render_graph.get_render_buffer(name);
}

render_texture &render_pass::add_texture(const std::string_view &name)
{
	return m_render_graph.get_render_texture(name);
}

void render_pass::set_execution(std::function<void(vulkan::command_buffer &)> f)
{
	m_execution_function = std::move(f);
}

render_graph::render_graph(vulkan::context &context)
    : m_context(context)
{
}

void render_graph::reset()
{
}

void render_graph::compile()
{
	UNUSED(m_context);
}

void render_graph::execute(vulkan::command_buffer &cmd_buf)
{
	std::vector<std::string_view> names;
	names.reserve(m_render_passes.size());
	for (const auto &[name, _] : m_render_passes)
	{
		names.push_back(name);
	}
	std::sort(names.begin(), names.end());

	for (const std::string_view name : names)
	{
		m_render_passes[name]->execute(cmd_buf);
	}
}

render_pass &render_graph::add_render_pass(const std::string_view &name)
{
	m_render_passes.emplace(name, make_uref<render_pass>(*this));
	return *m_render_passes[name];
}

render_texture &render_graph::get_render_texture(const std::string_view &name)
{
	if (m_render_textures.contains(name))
	{
		return *m_render_textures[name];
	}
	else
	{
		m_render_textures.emplace(name, make_uref<render_texture>());
		return *m_render_textures[name];
	}
}

render_buffer &render_graph::get_render_buffer(const std::string_view &name)
{
	if (m_render_buffers.contains(name))
	{
		return *m_render_buffers[name];
	}
	else
	{
		m_render_buffers.emplace(name, make_uref<render_buffer>());
		return *m_render_buffers[name];
	}
}
