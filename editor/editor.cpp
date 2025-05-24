#include <platform/input.h>

#include "editor.h"

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

void editor::build()
{
	m_context.build();
	m_ui.build(*this);

	logger::register_logger(&m_logger);
	input::register_window(m_context.m_window.m_window);

	build_default_settings();
	m_scene.build(m_context, m_settings);
}

void editor::update()
{
	m_ui.generate_frame();
	m_scene.update(m_settings);
}

void editor::draw_ui(vulkan::command_buffer &command_buffer)
{
	command_buffer.transition_image_layout(
	    m_scene.m_framebuffer.m_resolve_texture->m_image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
	    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
	    VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

	const VkRenderingAttachmentInfo resolve_attachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,                        //
		.pNext = nullptr,                                                            //
		.imageView = m_scene.m_framebuffer.m_resolve_texture->m_image_view.m_handle, //
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,                     //
		.resolveMode = VK_RESOLVE_MODE_NONE,                                         //
		.resolveImageView = VK_NULL_HANDLE,                                          //
		.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,                             //
		.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,                                        //
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,                                     //
		.clearValue = {},                                                            //
	};
	const VkRenderingInfo rendering_info_2 = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO, //
		.pNext = nullptr,                          //
		.flags = 0,                                //
		.renderArea = { { 0, 0 },
		                { m_scene.m_framebuffer.m_color_texture->m_image.m_info.m_width,
		                  m_scene.m_framebuffer.m_color_texture->m_image.m_info.m_height } }, //
		.layerCount = 1,                                                                      //
		.viewMask = 0,                                                                        //
		.colorAttachmentCount = 1,                                                            //
		.pColorAttachments = &resolve_attachment,                                             //
		.pDepthAttachment = VK_NULL_HANDLE,                                                   //
		.pStencilAttachment = nullptr,                                                        //
	};
	vkCmdBeginRendering(command_buffer.m_handle, &rendering_info_2);
	{
		m_ui.draw(command_buffer);
	}
	vkCmdEndRendering(command_buffer.m_handle);
}

void editor::draw_(vulkan::command_buffer &command_buffer)
{
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

void editor::draw()
{
	vulkan::command_buffer &command_buffer = m_context.begin_frame();
	{
		m_scene.draw(command_buffer); /* (TODO, thoave01): Move into here. */

		VkViewport viewport = {};
		viewport.x = m_settings.viewport_x;
		viewport.y = (float)(m_settings.viewport_y + m_settings.viewport_height);
		viewport.width = (float)m_settings.viewport_width;
		viewport.height = -(float)m_settings.viewport_height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor = {};
		scissor.offset = { m_settings.viewport_x, m_settings.viewport_y };
		scissor.extent = { m_settings.viewport_width, m_settings.viewport_height };
		vkCmdSetViewport(command_buffer.m_handle, 0, 1, &viewport);
		vkCmdSetScissor(command_buffer.m_handle, 0, 1, &scissor);

		command_buffer.transition_image_layout(m_scene.m_framebuffer.m_color_texture->m_image,
		                                       VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, 0,
		                                       VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0);
		command_buffer.transition_image_layout(m_scene.m_framebuffer.m_resolve_texture->m_image,
		                                       VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, 0,
		                                       VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0);

		if (m_settings.sample_count != VK_SAMPLE_COUNT_1_BIT)
		{
			VkClearValue clear_color = {};
			clear_color.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
			const VkRenderingAttachmentInfo color_attachment = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,                               //
				.pNext = nullptr,                                                                   //
				.imageView = m_scene.m_framebuffer.m_color_texture->m_image_view.m_handle,          //
				.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,                            //
				.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT,                                         //
				.resolveImageView = m_scene.m_framebuffer.m_resolve_texture->m_image_view.m_handle, //
				.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,                     //
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                                              //
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,                                        //
				.clearValue = clear_color,                                                          //
			};
			VkClearValue clear_depth = {};
			clear_depth.depthStencil = { 1.0f, 0 };
			const VkRenderingAttachmentInfo depth_attachment = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,                      //
				.pNext = nullptr,                                                          //
				.imageView = m_scene.m_framebuffer.m_depth_texture->m_image_view.m_handle, //
				.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,                   //
				.resolveMode = VK_RESOLVE_MODE_NONE,                                       //
				.resolveImageView = VK_NULL_HANDLE,                                        //
				.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,                           //
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                                     //
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,                               //
				.clearValue = clear_depth,                                                 //
			};
			const VkRenderingInfo rendering_info = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_INFO, //
				.pNext = nullptr,                          //
				.flags = 0,                                //
				.renderArea = { { 0, 0 },
				                { m_scene.m_framebuffer.m_color_texture->m_image.m_info.m_width,
				                  m_scene.m_framebuffer.m_color_texture->m_image.m_info.m_height } }, //
				.layerCount = 1,                                                                      //
				.viewMask = 0,                                                                        //
				.colorAttachmentCount = 1,                                                            //
				.pColorAttachments = &color_attachment,                                               //
				.pDepthAttachment = &depth_attachment,                                                //
				.pStencilAttachment = nullptr,                                                        //
			};

			/* Render. */
			vkCmdBeginRendering(command_buffer.m_handle, &rendering_info);
			{
				draw_(command_buffer);
			}
			vkCmdEndRendering(command_buffer.m_handle);
		}
		else
		{
			VkClearValue clear_color = {};
			clear_color.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
			const VkRenderingAttachmentInfo color_attachment = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,                        //
				.pNext = nullptr,                                                            //
				.imageView = m_scene.m_framebuffer.m_resolve_texture->m_image_view.m_handle, //
				.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,                     //
				.resolveMode = VK_RESOLVE_MODE_NONE,                                         //
				.resolveImageView = VK_NULL_HANDLE,                                          //
				.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,                             //
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                                       //
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,                                     //
				.clearValue = clear_color,                                                   //
			};
			VkClearValue clear_depth = {};
			clear_depth.depthStencil = { 1.0f, 0 };
			const VkRenderingAttachmentInfo depth_attachment = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,                      //
				.pNext = nullptr,                                                          //
				.imageView = m_scene.m_framebuffer.m_depth_texture->m_image_view.m_handle, //
				.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,                   //
				.resolveMode = VK_RESOLVE_MODE_NONE,                                       //
				.resolveImageView = VK_NULL_HANDLE,                                        //
				.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,                           //
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                                     //
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,                               //
				.clearValue = clear_depth,                                                 //
			};
			const VkRenderingInfo rendering_info = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_INFO, //
				.pNext = nullptr,                          //
				.flags = 0,                                //
				.renderArea = { { 0, 0 },
				                { m_scene.m_framebuffer.m_color_texture->m_image.m_info.m_width,
				                  m_scene.m_framebuffer.m_color_texture->m_image.m_info.m_height } }, //
				.layerCount = 1,                                                                      //
				.viewMask = 0,                                                                        //
				.colorAttachmentCount = 1,                                                            //
				.pColorAttachments = &color_attachment,                                               //
				.pDepthAttachment = &depth_attachment,                                                //
				.pStencilAttachment = nullptr,                                                        //
			};

			/* Render. */
			vkCmdBeginRendering(command_buffer.m_handle, &rendering_info);
			{
				draw_(command_buffer);
			}
			vkCmdEndRendering(command_buffer.m_handle);
		}

		draw_ui(command_buffer);
	}
	m_context.end_frame(*m_scene.m_framebuffer.m_resolve_texture);
}
