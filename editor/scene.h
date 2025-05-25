#pragma once

#include <vector>

#include <assets/image.h>
#include <assets/model.h>
#include <renderer/vulkan/buffer.h>
#include <renderer/vulkan/command_buffer.h>
#include <renderer/vulkan/context.h>
#include <renderer/vulkan/image.h>
#include <utils/util.h>

#include "object.h"
#include "settings.h"

using entity = u64;
template <typename T> using estorage = std::unordered_map<entity, T>;

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

struct scene_uniforms
{
	glm::mat4 view;
	glm::mat4 projection;
	u32 enable_mipmapping;
};
static_assert(sizeof(scene_uniforms) == 4 * 4 * 4 * 2 + sizeof(u32), "Unexpected scene struct uniform size");

class scene
{
public:
	scene() = default;
	~scene() = default;

	scene(const scene &) = delete;
	scene operator=(const scene &) = delete;

	void build(vulkan::context &context, const settings &settings);
	void update(vulkan::context &context, const settings &settings);
	void draw_(vulkan::command_buffer &command_buffer, const settings &settings);
	void draw(vulkan::command_buffer &command_buffer, const settings &settings);

	camera m_camera = {};
	scene_uniforms m_uniforms = {};
	vulkan::buffer m_uniform_buffer = {};
	vulkan::pipeline *m_default_pipeline = nullptr;

	struct
	{
		ref<vulkan::texture> m_color_texture = make_ref<vulkan::texture>();
		ref<vulkan::texture> m_depth_texture = make_ref<vulkan::texture>();
		ref<vulkan::texture> m_resolve_texture = make_ref<vulkan::texture>();
	} m_framebuffer;

	entity create_entity();

	estorage<ref<static_mesh>> m_static_mesh_storage = {};
	estorage<ref<skybox>> m_skybox_storage = {};

	struct
	{
		u32 m_vertex_count = 0;
		vulkan::pipeline m_pipeline = {};
		vulkan::buffer m_vertex_buffer = {};
	} m_grid;

	struct
	{
		u32 m_vertex_count = 6;
		vulkan::pipeline m_pipeline = {};
		vulkan::buffer m_vertex_buffer = {};
	} m_plane;

private:
	entity m_entity = 0;
};
