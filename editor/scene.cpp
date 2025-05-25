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

	/* Framebuffer. */
	m_framebuffer.m_color_texture->build(
	    context, { .m_format = settings.color_format,
	               .m_width = settings.viewport_width,
	               .m_height = settings.viewport_height,
	               .m_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
	               .m_sample_count = settings.sample_count });
	m_framebuffer.m_depth_texture->build(context, { .m_format = VK_FORMAT_D32_SFLOAT,
	                                                .m_width = settings.viewport_width,
	                                                .m_height = settings.viewport_width,
	                                                .m_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	                                                .m_sample_count = settings.sample_count });
	m_framebuffer.m_resolve_texture->build(
	    context, { .m_format = settings.color_format,
	               .m_width = settings.viewport_width,
	               .m_height = settings.viewport_height,
	               .m_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT });

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
}

void scene::update(vulkan::context &context, const settings &settings)
{
	/* Update camera. */
	m_camera.update((float)settings.viewport_width / (float)settings.viewport_height);

	/* Update uniforms. */
	m_uniforms.view = m_camera.m_view;
	m_uniforms.projection = m_camera.m_projection;
	m_uniforms.enable_mipmapping = settings.enable_mipmapping;
	m_uniform_buffer.fill(&m_uniforms, sizeof(m_uniforms));

	/* Update framebuffer. */
	bool width_changed = settings.viewport_width != m_framebuffer.m_color_texture->m_image.m_info.m_width;
	bool height_changed = settings.viewport_height != m_framebuffer.m_color_texture->m_image.m_info.m_height;
	bool sample_count_changed = settings.sample_count != m_framebuffer.m_color_texture->m_image.m_info.m_sample_count;
	if (width_changed | height_changed | sample_count_changed)
	{
		m_framebuffer.m_color_texture = make_ref<vulkan::texture>();
		m_framebuffer.m_color_texture->build(
		    context, { .m_format = settings.color_format,
		               .m_width = settings.viewport_width,
		               .m_height = settings.viewport_height,
		               .m_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
		               .m_sample_count = settings.sample_count });
		m_framebuffer.m_depth_texture = make_ref<vulkan::texture>();
		m_framebuffer.m_depth_texture->build(context, { .m_format = VK_FORMAT_D32_SFLOAT,
		                                                .m_width = settings.viewport_width,
		                                                .m_height = settings.viewport_height,
		                                                .m_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		                                                .m_sample_count = settings.sample_count });
		m_framebuffer.m_resolve_texture = make_ref<vulkan::texture>();
		m_framebuffer.m_resolve_texture->build(
		    context, { .m_format = settings.color_format,
		               .m_width = settings.viewport_width,
		               .m_height = settings.viewport_height,
		               .m_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT });
	}

	/* Update materials. */
	if (sample_count_changed)
	{
		for (auto &[e, static_mesh] : m_static_mesh_storage)
		{
			static_mesh->update_material(settings.sample_count);
		}
		for (auto &[e, skybox] : m_skybox_storage)
		{
			skybox->update_material(settings.sample_count);
		}
		m_grid.m_pipeline.set_sample_count(settings.sample_count);
		m_grid.m_pipeline.update();
		m_plane.m_pipeline.set_sample_count(settings.sample_count);
		m_plane.m_pipeline.update();
	}
}

void scene::draw_(vulkan::command_buffer &command_buffer, const settings &settings)
{
	/* (TODO, thoave01): Add some sort of default pipeline with scene defaults and pipeline compatibility. */
	command_buffer.bind_pipeline(*m_default_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
	command_buffer.set_uniform_buffer(0, m_uniform_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);

	for (auto &[e, static_mesh] : m_static_mesh_storage)
	{
		static_mesh->draw(command_buffer);
	}
	for (auto &[e, skybox] : m_skybox_storage)
	{
		if (settings.enable_skybox)
		{
			skybox->draw(command_buffer);
		}
	}
	if (settings.enable_grid)
	{
		/* Draw grid. */
		command_buffer.bind_pipeline(m_grid.m_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
		constexpr VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(command_buffer.m_handle, 0, 1, &m_grid.m_vertex_buffer.m_handle, &offset);
		command_buffer.set_uniform_buffer(0, m_uniform_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
		vkCmdDraw(command_buffer.m_handle, m_grid.m_vertex_count, 1, 0, 0);

		/* Draw plane. */
		command_buffer.bind_pipeline(m_plane.m_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
		vkCmdBindVertexBuffers(command_buffer.m_handle, 0, 1, &m_plane.m_vertex_buffer.m_handle, &offset);
		command_buffer.set_uniform_buffer(0, m_uniform_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
		vkCmdDraw(command_buffer.m_handle, 6, 1, 0, 0);
	}
}

void scene::draw(vulkan::command_buffer &command_buffer, const settings &settings)
{
	const u32 render_width = m_framebuffer.m_color_texture->m_image.m_info.m_width;
	const u32 render_height = m_framebuffer.m_color_texture->m_image.m_info.m_height;

	VkViewport viewport = { 0.0f, (float)render_height, (float)render_width, -(float)render_height, 0.0f, 1.0f };
	VkRect2D scissor = { { 0.0f, 0.0f }, { render_width, render_height } };
	vkCmdSetViewport(command_buffer.m_handle, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer.m_handle, 0, 1, &scissor);

	command_buffer.transition_image_layout(m_framebuffer.m_color_texture->m_image, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
	                                       VK_PIPELINE_STAGE_2_NONE, 0, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
	                                       VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
	command_buffer.transition_image_layout(m_framebuffer.m_resolve_texture->m_image, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
	                                       VK_PIPELINE_STAGE_2_NONE, 0, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
	                                       VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

	if (settings.sample_count != VK_SAMPLE_COUNT_1_BIT)
	{
		VkClearValue clear_color = {};
		clear_color.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		const VkRenderingAttachmentInfo color_attachment = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,                       //
			.pNext = nullptr,                                                           //
			.imageView = m_framebuffer.m_color_texture->m_image_view.m_handle,          //
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,                    //
			.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT,                                 //
			.resolveImageView = m_framebuffer.m_resolve_texture->m_image_view.m_handle, //
			.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,             //
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                                      //
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,                                //
			.clearValue = clear_color,                                                  //
		};
		VkClearValue clear_depth = {};
		clear_depth.depthStencil = { 1.0f, 0 };
		const VkRenderingAttachmentInfo depth_attachment = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,              //
			.pNext = nullptr,                                                  //
			.imageView = m_framebuffer.m_depth_texture->m_image_view.m_handle, //
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,           //
			.resolveMode = VK_RESOLVE_MODE_NONE,                               //
			.resolveImageView = VK_NULL_HANDLE,                                //
			.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,                   //
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                             //
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,                       //
			.clearValue = clear_depth,                                         //
		};
		const VkRenderingInfo rendering_info = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO, //
			.pNext = nullptr,                          //
			.flags = 0,                                //
			.renderArea = { { 0, 0 },
			                { m_framebuffer.m_color_texture->m_image.m_info.m_width,
			                  m_framebuffer.m_color_texture->m_image.m_info.m_height } }, //
			.layerCount = 1,                                                              //
			.viewMask = 0,                                                                //
			.colorAttachmentCount = 1,                                                    //
			.pColorAttachments = &color_attachment,                                       //
			.pDepthAttachment = &depth_attachment,                                        //
			.pStencilAttachment = nullptr,                                                //
		};

		/* Render. */
		vkCmdBeginRendering(command_buffer.m_handle, &rendering_info);
		{
			draw_(command_buffer, settings);
		}
		vkCmdEndRendering(command_buffer.m_handle);
	}
	else
	{
		VkClearValue clear_color = {};
		clear_color.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		const VkRenderingAttachmentInfo color_attachment = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,                //
			.pNext = nullptr,                                                    //
			.imageView = m_framebuffer.m_resolve_texture->m_image_view.m_handle, //
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,             //
			.resolveMode = VK_RESOLVE_MODE_NONE,                                 //
			.resolveImageView = VK_NULL_HANDLE,                                  //
			.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,                     //
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                               //
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,                             //
			.clearValue = clear_color,                                           //
		};
		VkClearValue clear_depth = {};
		clear_depth.depthStencil = { 1.0f, 0 };
		const VkRenderingAttachmentInfo depth_attachment = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,              //
			.pNext = nullptr,                                                  //
			.imageView = m_framebuffer.m_depth_texture->m_image_view.m_handle, //
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,           //
			.resolveMode = VK_RESOLVE_MODE_NONE,                               //
			.resolveImageView = VK_NULL_HANDLE,                                //
			.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,                   //
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                             //
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,                       //
			.clearValue = clear_depth,                                         //
		};
		const VkRenderingInfo rendering_info = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO, //
			.pNext = nullptr,                          //
			.flags = 0,                                //
			.renderArea = { { 0, 0 },
			                { m_framebuffer.m_color_texture->m_image.m_info.m_width,
			                  m_framebuffer.m_color_texture->m_image.m_info.m_height } }, //
			.layerCount = 1,                                                              //
			.viewMask = 0,                                                                //
			.colorAttachmentCount = 1,                                                    //
			.pColorAttachments = &color_attachment,                                       //
			.pDepthAttachment = &depth_attachment,                                        //
			.pStencilAttachment = nullptr,                                                //
		};

		/* Render. */
		vkCmdBeginRendering(command_buffer.m_handle, &rendering_info);
		{
			draw_(command_buffer, settings);
		}
		vkCmdEndRendering(command_buffer.m_handle);
	}
}

entity scene::create_entity()
{
	return m_entity++;
}
