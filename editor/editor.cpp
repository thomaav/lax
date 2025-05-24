#include "editor.h"

void editor::build()
{
	logger::register_logger(&m_logger);

	m_context.build();
	m_ui.build(*this);

	build_default_settings();
	m_scene.build(m_context, m_settings);
}

void editor::draw(vulkan::command_buffer &command_buffer)
{
	/* (TODO, thoave01): Camera object. */
	m_scene.m_uniforms.view = m_scene.m_camera.m_view;
	m_scene.m_uniforms.projection = m_scene.m_camera.m_projection;
	m_scene.m_uniforms.enable_mipmapping = m_settings.enable_mipmapping;
	m_scene.m_uniform_buffer.fill(&m_scene.m_uniforms, sizeof(m_scene.m_uniforms));

	/* (TODO, thoave01): Add some sort of default pipeline with scene defaults and pipeline compatibility. */
	command_buffer.bind_pipeline(*m_scene.m_default_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
	command_buffer.set_uniform_buffer(0, m_scene.m_uniform_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);

	for (auto &[e, static_mesh] : m_scene.m_static_mesh_storage)
	{
		static_mesh->draw(command_buffer);
	}
	for (auto &[e, skybox] : m_scene.m_skybox_storage)
	{
		if (m_settings.enable_skybox)
		{
			skybox->draw(command_buffer);
		}
	}
	if (m_settings.enable_grid)
	{
		/* Draw grid. */
		command_buffer.bind_pipeline(m_scene.m_grid.m_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
		constexpr VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(command_buffer.m_handle, 0, 1, &m_scene.m_grid.m_vertex_buffer.m_handle, &offset);
		command_buffer.set_uniform_buffer(0, m_scene.m_uniform_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
		vkCmdDraw(command_buffer.m_handle, m_scene.m_grid.m_vertex_count, 1, 0, 0);

		/* Draw plane. */
		command_buffer.bind_pipeline(m_scene.m_plane.m_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
		vkCmdBindVertexBuffers(command_buffer.m_handle, 0, 1, &m_scene.m_plane.m_vertex_buffer.m_handle, &offset);
		command_buffer.set_uniform_buffer(0, m_scene.m_uniform_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
		vkCmdDraw(command_buffer.m_handle, 6, 1, 0, 0);
	}
}

void editor::build_default_settings()
{
	/* No dependencies. */
	m_settings.enable_mipmapping = true;
	m_settings.enable_skybox = true;
	m_settings.enable_grid = true;
	m_settings.sample_count = VK_SAMPLE_COUNT_4_BIT;

	/* Dependencies. */
	if (VK_NULL_HANDLE != m_context.m_wsi.m_swapchain.m_handle)
	{
		m_settings.color_format = m_context.m_wsi.m_swapchain.m_images[0]->m_info.m_format;
		m_settings.depth_format = VK_FORMAT_D32_SFLOAT;
	}
	else
	{
		logger::warn("Swapchain not initialized when setting default framebuffer formats, using defaults");
		m_settings.color_format = VK_FORMAT_B8G8R8A8_SRGB;
		m_settings.depth_format = VK_FORMAT_D32_SFLOAT;
	}
}
