#include <chrono>

#include "scene.h"

void node::add_child(const ref<object> &child)
{
	node child_node = {};
	child_node.m_object = child;
	m_children.push_back(child_node);
}

void scene::build_default_scene(vulkan::context &context, const vulkan::render_pass &render_pass)
{
	ref<assets::model> model = make_ref<assets::model>();
	model->load("bin/assets/models/DamagedHelmet.glb");
	ref<static_mesh> static_mesh_ = make_ref<static_mesh>();
	static_mesh_->build(context, render_pass, model);
	m_root.add_child(std::static_pointer_cast<object>(static_mesh_));

	ref<skybox> skybox_ = make_ref<skybox>();
	skybox_->build(context, render_pass);
	m_root.add_child(std::static_pointer_cast<object>(skybox_));

	m_skybox = skybox_;
	m_static_mesh = static_mesh_;

	int width = 0;
	int height = 0;
	context.m_window.get_framebuffer_size(width, height);
	/* (TODO, thoave01): Not correct. Updated during main loop by docking. */
	m_camera.m_aspect = (float)width / (float)height;

	m_uniform_buffer =
	    context.m_resource_allocator.allocate_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(m_uniforms));
}
