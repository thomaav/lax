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

render_texture &render_pass::add_color_texture(const std::string_view &name)
{
	m_read_textures.push_back(name);
	m_written_textures.push_back(name);

	render_texture &rt = m_render_graph.get_render_texture(name);
	rt.m_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	return rt;
}

render_texture &render_pass::add_color_texture(const std::string_view &name, const render_texture_info &info)
{
	m_written_textures.push_back(name);

	render_texture &rt = m_render_graph.get_render_texture(name, info);
	rt.m_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	return rt;
}

render_texture &render_pass::add_depth_stencil_texture(const std::string_view &name, const render_texture_info &info)
{
	m_written_textures.push_back(name);

	render_texture &rt = m_render_graph.get_render_texture(name, info);
	rt.m_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	return rt;
}

render_texture &render_pass::add_resolve_texture(const std::string_view &name, const render_texture_info &info)
{
	m_written_textures.push_back(name);

	render_texture &rt = m_render_graph.get_render_texture(name, info);
	rt.m_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	return rt;
}

render_texture &render_pass::add_transfer_src_texture(const std::string_view &name)
{
	m_read_textures.push_back(name);

	render_texture &rt = m_render_graph.get_render_texture(name);
	rt.m_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	return rt;
}

render_texture &render_pass::add_transfer_dst_texture(const std::string_view &name, const render_texture_info &info)
{
	m_written_textures.push_back(name);

	render_texture &rt = m_render_graph.get_render_texture(name, info);
	rt.m_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	return rt;
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
	for (const auto &[name, render_texture] : m_render_textures)
	{
		render_texture->m_texture->build(m_context, { .m_format = render_texture->m_info.format,
		                                              .m_width = render_texture->m_info.width,
		                                              .m_height = render_texture->m_info.height,
		                                              .m_usage = render_texture->m_usage,
		                                              .m_sample_count = render_texture->m_info.sample_count });
	}
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
	assert_if(!m_render_textures.contains(name), "Requested texture %s does not exist");
	return *m_render_textures[name];
}

render_texture &render_graph::get_render_texture(const std::string_view &name, const render_texture_info &info)
{
	if (m_render_textures.contains(name))
	{
		assert_if(m_render_textures[name]->m_info != info, "Existing render texture %s info differs", name);
		return *m_render_textures[name];
	}
	else
	{
		m_render_textures.emplace(name, make_uref<render_texture>());
		m_render_textures[name]->m_info = info;
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
