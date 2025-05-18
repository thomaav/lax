#pragma once

#include <vector>

#include <assets/image.h>
#include <assets/model.h>
#include <renderer/vulkan/buffer.h>
#include <renderer/vulkan/command_buffer.h>
#include <renderer/vulkan/context.h>
#include <renderer/vulkan/image.h>
#include <utils/util.h>

struct scene_uniforms
{
	glm::mat4 view;
	glm::mat4 projection;
	/* (TODO, thoave01): Don't enable mipmapping at shader level. */
	u32 enable_mipmapping;
};
static_assert(sizeof(scene_uniforms) == 4 * 4 * 4 * 2 + sizeof(u32), "Unexpected scene struct uniform size");

struct object_uniforms
{
	glm::mat4 model;
};
static_assert(sizeof(object_uniforms) == 4 * 4 * 4, "Unexpected object struct uniform size");

class object
{
public:
	object() = default;
	virtual ~object() = default;

	virtual void draw(vulkan::command_buffer &command_buffer) = 0;

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
	void draw(vulkan::command_buffer &command_buffer) override;
	void update_material(const vulkan::render_pass &render_pass, VkSampleCountFlagBits sample_count);

	assets::image m_asset_image = {};
	vulkan::texture m_texture = {};
	vulkan::pipeline m_pipeline = {};
	object_uniforms m_uniforms = {};

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
	void draw(vulkan::command_buffer &command_buffer) override;
	void update_material(const vulkan::render_pass &render_pass, VkSampleCountFlagBits sample_count);

	ref<assets::model> m_model;
	vulkan::buffer m_vertex_buffer = {};
	u32 m_index_count = 0;
	vulkan::buffer m_index_buffer = {};
	vulkan::texture m_diffuse_texture = {};
	vulkan::pipeline m_pipeline = {};
	object_uniforms m_uniforms = {};

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

	scene_uniforms m_uniforms = {};
	vulkan::buffer m_uniform_buffer = {};

private:
};
