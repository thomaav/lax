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

void scene::build(vulkan::context &context, const settings &settings)
{
	/* Scene uniforms. */
	m_uniform_buffer =
	    context.m_resource_allocator.allocate_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(m_uniforms));

	/* Camera. */
	const glm::vec3 camera_position = glm::vec3(3.0f, 2.0f, 5.0f);
	const glm::vec3 camera_target = glm::vec3(0.0f);
	m_camera.build(camera_position, camera_target);

	/* Static mesh objects. */
	ref<assets::model> model = make_ref<assets::model>();
	model->load("bin/assets/models/DamagedHelmet.glb");
	for (int x = -2; x <= 2; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			ref<static_mesh> new_static_mesh = make_ref<static_mesh>();
			new_static_mesh->build(context, model);
			new_static_mesh->m_uniforms.model =
			    glm::translate(new_static_mesh->m_uniforms.model, glm::vec3((float)x * 2.0f, 0.0f, (float)y * 2.0f));

			entity e = create_entity();
			m_static_mesh_storage[e] = new_static_mesh;
		}
	}

	/* Skybox object. */
	entity skybox_e = create_entity();
	m_skybox_storage[skybox_e] = make_ref<skybox>();
	m_skybox_storage[skybox_e]->build(context);
	m_default_pipeline = &m_skybox_storage[skybox_e]->m_pipeline;

	/* Grid. */
	ref<assets::model> grid_model = make_ref<assets::model>();
	grid_model->generate_grid();

	m_grid.m_vertex_buffer = context.m_resource_allocator.allocate_buffer(
	    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	    sizeof(grid_model->m_meshes[0].m_vertices[0]) * grid_model->m_meshes[0].m_vertices.size());
	m_grid.m_vertex_buffer.fill(grid_model->m_meshes[0].m_vertices.data(),
	                            sizeof(grid_model->m_meshes[0].m_vertices[0]) *
	                                grid_model->m_meshes[0].m_vertices.size());
	m_grid.m_vertex_count = grid_model->m_meshes[0].m_vertices.size();

	m_grid.m_pipeline.add_shader(context.m_device, VK_SHADER_STAGE_VERTEX_BIT, "bin/assets/shaders/grid.vert.spv");
	m_grid.m_pipeline.add_shader(context.m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "bin/assets/shaders/grid.frag.spv");
	m_grid.m_pipeline.set_sample_count(settings.sample_count);
	m_grid.m_pipeline.set_cull_mode(VK_CULL_MODE_NONE);
	m_grid.m_pipeline.build(context.m_device);

	/* Plane. */
	ref<assets::model> plane_model = make_ref<assets::model>();
	plane_model->generate_plane();

	m_plane.m_vertex_buffer = context.m_resource_allocator.allocate_buffer(
	    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	    sizeof(plane_model->m_meshes[0].m_vertices[0]) * plane_model->m_meshes[0].m_vertices.size());
	m_plane.m_vertex_buffer.fill(plane_model->m_meshes[0].m_vertices.data(),
	                             sizeof(plane_model->m_meshes[0].m_vertices[0]) *
	                                 plane_model->m_meshes[0].m_vertices.size());
	m_plane.m_vertex_count = plane_model->m_meshes[0].m_vertices.size();

	m_plane.m_pipeline.add_shader(context.m_device, VK_SHADER_STAGE_VERTEX_BIT, "bin/assets/shaders/plane.vert.spv");
	m_plane.m_pipeline.add_shader(context.m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "bin/assets/shaders/plane.frag.spv");
	m_plane.m_pipeline.set_sample_count(settings.sample_count);
	m_plane.m_pipeline.set_cull_mode(VK_CULL_MODE_NONE);
	m_plane.m_pipeline.set_blend_enable(VK_TRUE);
	m_plane.m_pipeline.build(context.m_device);

	/* (TODO, thoave01): Updates based on settings, should be part of initialization. */
	for (auto &[e, static_mesh] : m_static_mesh_storage)
	{
		static_mesh->update_material(settings.sample_count);
	}
	for (auto &[e, skybox] : m_skybox_storage)
	{
		skybox->update_material(settings.sample_count);
	}

	/* Framebuffer. */
	m_framebuffer.m_color_texture->build(
	    context, { .m_format = settings.color_format,
	               .m_width = context.m_wsi.m_swapchain.m_extent.width,
	               .m_height = context.m_wsi.m_swapchain.m_extent.height,
	               .m_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
	               .m_sample_count = settings.sample_count });
	m_framebuffer.m_depth_texture->build(context, { .m_format = VK_FORMAT_D32_SFLOAT,
	                                                .m_width = context.m_wsi.m_swapchain.m_extent.width,
	                                                .m_height = context.m_wsi.m_swapchain.m_extent.height,
	                                                .m_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	                                                .m_sample_count = settings.sample_count });
	m_framebuffer.m_resolve_texture->build(
	    context, { .m_format = settings.color_format,
	               .m_width = context.m_wsi.m_swapchain.m_extent.width,
	               .m_height = context.m_wsi.m_swapchain.m_extent.height,
	               .m_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT });
}

void scene::update(const settings &settings)
{
	/* Update camera. */
	m_camera.update((float)settings.viewport_width / (float)settings.viewport_height);

	/* Update uniforms. */
	m_uniforms.view = m_camera.m_view;
	m_uniforms.projection = m_camera.m_projection;
	m_uniforms.enable_mipmapping = settings.enable_mipmapping;
	m_uniform_buffer.fill(&m_uniforms, sizeof(m_uniforms));
}

void scene::draw(vulkan::command_buffer &command_buffer)
{
}

entity scene::create_entity()
{
	return m_entity++;
}
