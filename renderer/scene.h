#pragma once

#include <vector>

#include <assets/image.h>
#include <assets/model.h>
#include <renderer/vulkan/buffer.h>
#include <renderer/vulkan/command_buffer.h>
#include <renderer/vulkan/context.h>
#include <renderer/vulkan/image.h>
#include <utils/util.h>

class object
{
public:
	object() = default;
	virtual ~object() = default;

	virtual void draw(vulkan::command_buffer &command_buffer, vulkan::buffer &uniform_buffer) const = 0;

private:
};

class skybox : public object
{
public:
	skybox() = default;
	~skybox() = default;

	skybox(const skybox &) = delete;
	skybox operator=(const skybox &) = delete;

	void build(vulkan::context &context, const vulkan::render_pass &render_pass);
	void draw(vulkan::command_buffer &command_buffer, vulkan::buffer &uniform_buffer) const override;
	void update_material(vulkan::context &context, const vulkan::render_pass &render_pass,
	                     VkSampleCountFlagBits sample_count);

	assets::image m_asset_image = {};
	vulkan::image m_image = {};
	vulkan::texture m_texture = {};
	vulkan::pipeline m_pipeline = {};

private:
};

class static_mesh : public object
{
public:
	static_mesh() = default;
	~static_mesh() = default;

	static_mesh(const static_mesh &) = delete;
	static_mesh operator=(const static_mesh &) = delete;

	void build(vulkan::context &context, const vulkan::render_pass &render_pass, ref<assets::model> model);
	void draw(vulkan::command_buffer &command_buffer, vulkan::buffer &uniform_buffer) const override;
	void update_material(vulkan::context &context, const vulkan::render_pass &render_pass,
	                     VkSampleCountFlagBits sample_count);

	ref<assets::model> m_model;
	vulkan::buffer m_vertex_buffer = {};
	u32 m_index_count = 0;
	vulkan::buffer m_index_buffer = {};
	vulkan::image m_diffuse_image = {}; /* (TODO, thoave01): Texture should contain image. */
	vulkan::texture m_diffuse_texture = {};
	vulkan::pipeline m_pipeline = {};

private:
};

class node
{
public:
	node() = default;
	~node() = default;

	node(const node &) = default;
	node operator=(const node &) = delete;

	void add_child(const ref<object> &child);

	ref<object> m_object = {};
	std::vector<node> m_children = {};

private:
};

class scene
{
public:
	scene() = default;
	~scene() = default;

	scene(const scene &) = delete;
	scene operator=(const scene &) = delete;

	void build_default_scene(vulkan::context &context, const vulkan::render_pass &render_pass);

	node m_root = {};

	/* (TODO, thoave01): Should go away. */
	ref<static_mesh> m_static_mesh = nullptr;
	ref<skybox> m_skybox = nullptr;

private:
};
