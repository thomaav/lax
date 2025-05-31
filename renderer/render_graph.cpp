#include "render_graph.h"

render_pass::render_pass(render_graph &rg)
    : m_render_graph(rg)
{
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

render_buffer &render_pass::add_buffer(const std::string_view &name)
{
	return m_render_graph.get_render_buffer(name);
}

render_texture &render_pass::add_texture(const std::string_view &name)
{
	return m_render_graph.get_render_texture(name);
}
