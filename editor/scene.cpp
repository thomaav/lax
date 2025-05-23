#include <chrono>

#include "scene.h"

entity create_entity()
{
	static entity e = 0;
	return e++;
}

void node::add_child(const ref<object> &child)
{
	node child_node = {};
	child_node.m_object = child;
	m_children.push_back(child_node);
}

void scene::build_default_scene(vulkan::context &context, const vulkan::render_pass &render_pass)
{
	m_uniform_buffer =
	    context.m_resource_allocator.allocate_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(m_uniforms));

	/* Static mesh objects. */
	ref<assets::model> model = make_ref<assets::model>();
	model->load("bin/assets/models/DamagedHelmet.glb");
	for (int x = -2; x <= 2; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			ref<static_mesh> new_static_mesh = make_ref<static_mesh>();
			new_static_mesh->build(context, render_pass, model);
			new_static_mesh->m_uniforms.model =
			    glm::translate(new_static_mesh->m_uniforms.model, glm::vec3((float)x * 2.0f, 0.0f, (float)y * 2.0f));

			entity e = create_entity();
			m_static_mesh_storage[e] = new_static_mesh;
		}
	}

	/* Skybox object. */
	entity skybox_e = create_entity();
	m_skybox_storage[skybox_e] = make_ref<skybox>();
	m_skybox_storage[skybox_e]->build(context, render_pass);
	m_default_pipeline = &m_skybox_storage[skybox_e]->m_pipeline;
}

entity scene::create_entity()
{
	return m_entity++;
}
