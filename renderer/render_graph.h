#pragma once

// clang-format off
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop
// clang-format on

#include <functional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <utils/type.h>

class render_graph;

namespace vulkan
{
class buffer;
class texture;
class command_buffer;
class context;
}

class render_resource
{
public:
	virtual ~render_resource() = default;
};

struct render_buffer_info
{
	auto operator<=>(const render_buffer_info &) const = default;
};

class render_buffer : public render_resource
{
public:
	render_buffer() = default;
	~render_buffer() = default;

	render_buffer(render_buffer &) = delete;
	render_buffer operator=(const render_buffer &) = delete;

	render_buffer_info m_info = {};
};

struct render_texture_info
{
	auto operator<=>(const render_texture_info &) const = default;

	VkFormat format;
	u32 width;
	u32 height;
	VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT;
};

class render_texture : public render_resource
{
public:
	render_texture() = default;
	~render_texture() = default;

	render_texture(render_texture &) = delete;
	render_texture operator=(const render_texture &) = delete;

	render_texture_info m_info = {};
	VkImageUsageFlags m_usage = {};
	uref<vulkan::texture> m_texture = make_uref<vulkan::texture>();
};

class render_pass
{
public:
	render_pass(render_graph &rg);
	~render_pass() = default;

	render_pass(render_pass &) = delete;
	render_pass operator=(const render_pass &) = delete;

	void execute(vulkan::command_buffer &cmd_buf);

	render_buffer &add_buffer(const std::string_view &name);

	render_texture &add_color_texture(const std::string_view &name);
	render_texture &add_color_texture(const std::string_view &name, const render_texture_info &info);
	render_texture &add_depth_stencil_texture(const std::string_view &name, const render_texture_info &info);
	render_texture &add_resolve_texture(const std::string_view &name, const render_texture_info &info);
	render_texture &add_transfer_src_texture(const std::string_view &name);
	render_texture &add_transfer_dst_texture(const std::string_view &name, const render_texture_info &info);

	void set_execution(std::function<void(vulkan::command_buffer &)> f);

private:
	/* Execution. */
	render_graph &m_render_graph;
	std::function<void(vulkan::command_buffer &)> m_execution_function;

	/* Resources. */
	std::vector<std::string_view> m_read_textures = {};
	std::vector<std::string_view> m_written_textures = {};
};

class render_graph
{
public:
	render_graph(vulkan::context &context);
	~render_graph() = default;

	render_graph(const render_graph &) = delete;
	render_graph operator=(const render_graph &) = delete;

	void reset();
	void compile();
	void execute(vulkan::command_buffer &cmd_buf);

	/* Render pass API. */
	render_pass &add_render_pass(const std::string_view &name);

	/* Resource API. */
	render_texture &get_render_texture(const std::string_view &name);
	render_texture &get_render_texture(const std::string_view &name, const render_texture_info &info);
	render_buffer &get_render_buffer(const std::string_view &name);

private:
	vulkan::context &m_context;

	/* Render passes. */
	std::unordered_map<std::string_view, uref<render_pass>> m_render_passes = {};

	/* Resources. */
	std::unordered_map<std::string_view, uref<render_buffer>> m_render_buffers = {};
	std::unordered_map<std::string_view, uref<render_texture>> m_render_textures = {};
};
