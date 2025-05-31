#pragma once

#include <string_view>
#include <unordered_map>
#include <vector>

#include <renderer/vulkan/buffer.h>
#include <renderer/vulkan/image.h>
#include <utils/type.h>

class render_graph;

using buffer = vulkan::buffer;
using texture = vulkan::texture;

class render_resource
{
public:
	virtual ~render_resource() = default;
};

class render_buffer : public render_resource
{
public:
	render_buffer() = default;
	~render_buffer() = default;

	render_buffer(render_buffer &) = delete;
	render_buffer operator=(const render_buffer &) = delete;
};

class render_texture : public render_resource
{
public:
	render_texture() = default;
	~render_texture() = default;

	render_texture(render_texture &) = delete;
	render_texture operator=(const render_texture &) = delete;
};

class render_pass
{
public:
	render_pass(render_graph &rg);
	~render_pass() = default;

	render_pass(render_pass &) = delete;
	render_pass operator=(const render_pass &) = delete;

	render_buffer &add_buffer(const std::string_view &name);
	render_texture &add_texture(const std::string_view &name);

private:
	render_graph &m_render_graph;

	std::vector<ref<buffer>> m_buffers = {};
	std::vector<ref<texture>> m_textures = {};
};

class render_graph
{
public:
	render_graph() = default;
	~render_graph() = default;

	render_graph(const render_graph &) = delete;
	render_graph operator=(const render_graph &) = delete;

	/* Render pass API. */
	render_pass &add_render_pass(const std::string_view &name);

	/* Resource API. */
	render_texture &get_render_texture(const std::string_view &name);
	render_buffer &get_render_buffer(const std::string_view &name);

private:
	/* Render passes. */
	std::unordered_map<std::string_view, uref<render_pass>> m_render_passes = {};

	/* Resources. */
	std::unordered_map<std::string_view, uref<render_buffer>> m_render_buffers = {};
	std::unordered_map<std::string_view, uref<render_texture>> m_render_textures = {};
};
