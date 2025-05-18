#pragma once

#include <vector>

#include <assets/image.h>
#include <assets/model.h>
#include <renderer/object.h>
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
	camera m_camera = {};
	scene_uniforms m_uniforms = {};
	vulkan::buffer m_uniform_buffer = {};

private:
};
