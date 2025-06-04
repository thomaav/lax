#include <platform/input.h>
#include <renderer/render_graph.h>

#include "editor.h"

void editor::build_default_settings()
{
	/* No dependencies. */
	m_settings.enable_mipmapping = true;
	m_settings.enable_skybox = true;
	m_settings.enable_grid = true;
	m_settings.sample_count = VK_SAMPLE_COUNT_4_BIT;

	m_settings.viewport_x = 0;
	m_settings.viewport_y = 0;
	m_settings.viewport_width = 800;
	m_settings.viewport_height = 600;

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
	logger::register_logger(&m_logger);

	m_context.build();
	m_ui.build(*this);

	input::register_window(m_context.m_window.m_window);

	build_default_settings();
	m_scene.build(m_context, m_settings);
}

void editor::update()
{
	m_ui.generate_frame();
	m_scene.update(m_context, m_settings);
}

void editor::draw()
{
	render_graph rg(m_context);
	rg.reset();
	{
		render_pass &rp1 = rg.add_render_pass("rp1");
		{
			render_texture &viewport_color =
			    rp1.add_color_texture("viewport_color", {
			                                                .format = m_settings.color_format,
			                                                .width = m_settings.viewport_width,
			                                                .height = m_settings.viewport_height,
			                                                .sample_count = m_settings.sample_count,
			                                            });
			render_texture &viewport_depth =
			    rp1.add_depth_stencil_texture("viewport_depth", {
			                                                        .format = m_settings.depth_format,
			                                                        .width = m_settings.viewport_width,
			                                                        .height = m_settings.viewport_height,
			                                                        .sample_count = m_settings.sample_count,
			                                                    });
			render_texture &viewport_resolve =
			    rp1.add_resolve_texture("viewport_resolve", { .format = m_settings.color_format,
			                                                  .width = m_settings.viewport_width,
			                                                  .height = m_settings.viewport_height });
			rp1.set_execution(
			    [&](vulkan::command_buffer &cmd_buf)
			    {
				    VkViewport viewport = { 0.0f,
					                        (float)m_settings.viewport_height,
					                        (float)m_settings.viewport_width,
					                        -(float)m_settings.viewport_height,
					                        0.0f,
					                        1.0f };
				    VkRect2D scissor = { { 0.0f, 0.0f }, { m_settings.viewport_width, m_settings.viewport_height } };
				    vkCmdSetViewport(cmd_buf.m_handle, 0, 1, &viewport);
				    vkCmdSetScissor(cmd_buf.m_handle, 0, 1, &scissor);

				    VkClearValue clear_color = {};
				    clear_color.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
				    const VkRenderingAttachmentInfo color_attachment = {
					    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,                  //
					    .pNext = nullptr,                                                      //
					    .imageView = viewport_color.m_texture->m_image_view.m_handle,          //
					    .imageLayout = VK_IMAGE_LAYOUT_GENERAL,                                //
					    .resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT,                            //
					    .resolveImageView = viewport_resolve.m_texture->m_image_view.m_handle, //
					    .resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL,                         //
					    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                                 //
					    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,                           //
					    .clearValue = clear_color,                                             //
				    };
				    VkClearValue clear_depth = {};
				    clear_depth.depthStencil = { 1.0f, 0 };
				    const VkRenderingAttachmentInfo depth_attachment = {
					    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,         //
					    .pNext = nullptr,                                             //
					    .imageView = viewport_depth.m_texture->m_image_view.m_handle, //
					    .imageLayout = VK_IMAGE_LAYOUT_GENERAL,                       //
					    .resolveMode = VK_RESOLVE_MODE_NONE,                          //
					    .resolveImageView = VK_NULL_HANDLE,                           //
					    .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,              //
					    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                        //
					    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,                  //
					    .clearValue = clear_depth,                                    //
				    };
				    const VkRenderingInfo rendering_info = {
					    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,                                             //
					    .pNext = nullptr,                                                                      //
					    .flags = 0,                                                                            //
					    .renderArea = { { 0, 0 }, { m_settings.viewport_width, m_settings.viewport_height } }, //
					    .layerCount = 1,                                                                       //
					    .viewMask = 0,                                                                         //
					    .colorAttachmentCount = 1,                                                             //
					    .pColorAttachments = &color_attachment,                                                //
					    .pDepthAttachment = &depth_attachment,                                                 //
					    .pStencilAttachment = nullptr,                                                         //
				    };

				    /* Render. */
				    vkCmdBeginRendering(cmd_buf.m_handle, &rendering_info);
				    {
					    cmd_buf.bind_pipeline(*m_scene.m_default_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
					    cmd_buf.set_uniform_buffer(0, m_scene.m_uniform_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);

					    for (auto &[e, static_mesh] : m_scene.m_static_mesh_storage)
					    {
						    static_mesh->draw(cmd_buf);
					    }
					    for (auto &[e, skybox] : m_scene.m_skybox_storage)
					    {
						    if (m_settings.enable_skybox)
						    {
							    skybox->draw(cmd_buf);
						    }
					    }
					    if (m_settings.enable_grid)
					    {
						    /* Draw grid. */
						    cmd_buf.bind_pipeline(m_scene.m_grid.m_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
						    constexpr VkDeviceSize offset = 0;
						    vkCmdBindVertexBuffers(cmd_buf.m_handle, 0, 1, &m_scene.m_grid.m_vertex_buffer.m_handle,
						                           &offset);
						    cmd_buf.set_uniform_buffer(0, m_scene.m_uniform_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
						    vkCmdDraw(cmd_buf.m_handle, m_scene.m_grid.m_vertex_count, 1, 0, 0);

						    /* Draw plane. */
						    cmd_buf.bind_pipeline(m_scene.m_plane.m_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
						    vkCmdBindVertexBuffers(cmd_buf.m_handle, 0, 1, &m_scene.m_plane.m_vertex_buffer.m_handle,
						                           &offset);
						    cmd_buf.set_uniform_buffer(0, m_scene.m_uniform_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
						    vkCmdDraw(cmd_buf.m_handle, 6, 1, 0, 0);
					    }
				    }
				    vkCmdEndRendering(cmd_buf.m_handle);
			    });
		}

		render_pass &rp2 = rg.add_render_pass("rp2");
		{
			render_texture &viewport_resolve = rp2.add_transfer_src_texture("viewport_resolve");
			render_texture &editor_color =
			    rp2.add_transfer_dst_texture("editor_color", {
			                                                     .format = m_settings.color_format,
			                                                     .width = m_context.m_wsi.m_swapchain.m_extent.width,
			                                                     .height = m_context.m_wsi.m_swapchain.m_extent.height,
			                                                 });
			rp2.set_execution(
			    [&](vulkan::command_buffer &cmd_buf)
			    {
				    // cmd_buf.transition_image_layout(
				    //     m_scene.m_framebuffer.m_resolve_texture->m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				    //     VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				    //     VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT);
				    // cmd_buf.transition_image_layout(
				    //     m_framebuffer.m_color_texture->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				    //     VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				    //     VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);

				    VkImageCopy copy_info = {};
				    copy_info.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				    copy_info.srcSubresource.mipLevel = 0;
				    copy_info.srcSubresource.baseArrayLayer = 0;
				    copy_info.srcSubresource.layerCount = 1;
				    copy_info.srcOffset = { 0, 0, 0 };
				    copy_info.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				    copy_info.dstSubresource.mipLevel = 0;
				    copy_info.dstSubresource.baseArrayLayer = 0;
				    copy_info.dstSubresource.layerCount = 1;
				    copy_info.dstOffset = { (int)m_settings.viewport_x, (int)m_settings.viewport_y, 0 };
				    copy_info.extent = { m_settings.viewport_width, m_settings.viewport_height, 1 };
				    vkCmdCopyImage(cmd_buf.m_handle, viewport_resolve.m_texture->m_image.m_handle,
				                   VK_IMAGE_LAYOUT_GENERAL, editor_color.m_texture->m_image.m_handle,
				                   VK_IMAGE_LAYOUT_GENERAL, 1, &copy_info);

				    // cmd_buf.transition_image_layout(m_framebuffer.m_color_texture->m_image,
				    //                                 VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
				    //                                 VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE, 0);
			    });
		}

		render_pass &rp3 = rg.add_render_pass("rp3");
		{
			render_texture &editor_color = rp3.add_color_texture("editor_color");
			rp3.set_execution(
			    [&](vulkan::command_buffer &cmd_buf)
			    {
				    const u32 render_width = editor_color.m_texture->m_image.m_info.m_width;
				    const u32 render_height = editor_color.m_texture->m_image.m_info.m_height;

				    VkViewport viewport = {
					    0.0f, (float)render_height, (float)render_width, -(float)render_height, 0.0f, 1.0f
				    };
				    VkRect2D scissor = { { 0.0f, 0.0f }, { render_width, render_height } };
				    vkCmdSetViewport(cmd_buf.m_handle, 0, 1, &viewport);
				    vkCmdSetScissor(cmd_buf.m_handle, 0, 1, &scissor);

				    // cmd_buf.transition_image_layout(
				    //     m_framebuffer.m_color_texture->m_image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				    //     VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				    //     VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				    //     VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

				    const VkRenderingAttachmentInfo color_attachment = {
					    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,       //
					    .pNext = nullptr,                                           //
					    .imageView = editor_color.m_texture->m_image_view.m_handle, //
					    .imageLayout = VK_IMAGE_LAYOUT_GENERAL,                     //
					    .resolveMode = VK_RESOLVE_MODE_NONE,                        //
					    .resolveImageView = VK_NULL_HANDLE,                         //
					    .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,            //
					    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,                       //
					    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,                    //
					    .clearValue = {},                                           //
				    };
				    const VkRenderingInfo rendering_info = {
					    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,                   //
					    .pNext = nullptr,                                            //
					    .flags = 0,                                                  //
					    .renderArea = { { 0, 0 }, { render_width, render_height } }, //
					    .layerCount = 1,                                             //
					    .viewMask = 0,                                               //
					    .colorAttachmentCount = 1,                                   //
					    .pColorAttachments = &color_attachment,                      //
					    .pDepthAttachment = VK_NULL_HANDLE,                          //
					    .pStencilAttachment = nullptr,                               //
				    };
				    vkCmdBeginRendering(cmd_buf.m_handle, &rendering_info);
				    {
					    m_ui.draw(cmd_buf);
				    }
				    vkCmdEndRendering(cmd_buf.m_handle);
			    });
		}
	}
	rg.compile();

	vulkan::command_buffer &command_buffer = m_context.begin_frame();
	{
		rg.execute(command_buffer);
	}
	m_context.end_frame(*rg.get_render_texture("editor_color").m_texture);
}
