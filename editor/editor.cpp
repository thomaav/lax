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

	/* (TODO, thoave01): Move into render graph passes. */
	m_framebuffer.m_color_texture->build(m_context,
	                                     { .m_format = m_settings.color_format,
	                                       .m_width = m_context.m_wsi.m_swapchain.m_extent.width,
	                                       .m_height = m_context.m_wsi.m_swapchain.m_extent.height,
	                                       .m_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
	                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	                                       .m_sample_count = VK_SAMPLE_COUNT_1_BIT });
}

void editor::update()
{
	m_ui.generate_frame();
	m_scene.update(m_context, m_settings);
}

void editor::draw_ui(vulkan::command_buffer &command_buffer)
{
	const u32 render_width = m_framebuffer.m_color_texture->m_image.m_info.m_width;
	const u32 render_height = m_framebuffer.m_color_texture->m_image.m_info.m_height;

	VkViewport viewport = { 0.0f, (float)render_height, (float)render_width, -(float)render_height, 0.0f, 1.0f };
	VkRect2D scissor = { { 0.0f, 0.0f }, { render_width, render_height } };
	vkCmdSetViewport(command_buffer.m_handle, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer.m_handle, 0, 1, &scissor);

	command_buffer.transition_image_layout(
	    m_framebuffer.m_color_texture->m_image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
	    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
	    VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

	const VkRenderingAttachmentInfo color_attachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,              //
		.pNext = nullptr,                                                  //
		.imageView = m_framebuffer.m_color_texture->m_image_view.m_handle, //
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,           //
		.resolveMode = VK_RESOLVE_MODE_NONE,                               //
		.resolveImageView = VK_NULL_HANDLE,                                //
		.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,                   //
		.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,                              //
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,                           //
		.clearValue = {},                                                  //
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
		.pDepthAttachment = VK_NULL_HANDLE,                                           //
		.pStencilAttachment = nullptr,                                                //
	};
	vkCmdBeginRendering(command_buffer.m_handle, &rendering_info);
	{
		m_ui.draw(command_buffer);
	}
	vkCmdEndRendering(command_buffer.m_handle);
}

void editor::draw()
{
	render_graph rg(m_context);
	rg.reset();
	{
		render_pass &rp1 = rg.add_render_pass("rp1");
		{
			rp1.add_color_texture("viewport_color", {
			                                            .format = m_settings.color_format,
			                                            .width = m_settings.viewport_width,
			                                            .height = m_settings.viewport_height,
			                                        });
			rp1.add_depth_stencil_texture("viewport_depth", { .format = m_settings.depth_format,
			                                                  .width = m_settings.viewport_width,
			                                                  .height = m_settings.viewport_height });
			rp1.add_resolve_texture("viewport_resolve", { .format = m_settings.color_format,
			                                              .width = m_settings.viewport_width,
			                                              .height = m_settings.viewport_height });
			rp1.set_execution([&](vulkan::command_buffer &cmd_buf) { UNUSED(cmd_buf); });
		}

		render_pass &rp2 = rg.add_render_pass("rp2");
		{
			rp2.add_transfer_src_texture("viewport_resolve");
			rp2.add_transfer_dst_texture("editor_color", {
			                                                 .format = m_settings.color_format,
			                                                 .width = m_context.m_wsi.m_swapchain.m_extent.width,
			                                                 .height = m_context.m_wsi.m_swapchain.m_extent.height,
			                                             });
			rp2.set_execution([&](vulkan::command_buffer &cmd_buf) { UNUSED(cmd_buf); });
		}

		render_pass &rp3 = rg.add_render_pass("rp3");
		{
			rp3.add_color_texture("editor_color");
			rp3.set_execution([&](vulkan::command_buffer &cmd_buf) { UNUSED(cmd_buf); });
		}
	}
	rg.compile();

	vulkan::command_buffer &command_buffer = m_context.begin_frame();
	{
		rg.execute(command_buffer);
	}
	m_context.end_frame(*m_framebuffer.m_color_texture);

	// vulkan::command_buffer &command_buffer = m_context.begin_frame();
	// {
	// 	m_scene.draw(command_buffer, m_settings);

	// 	/* Copy scene framebuffer to editor framebuffer. */
	// 	command_buffer.transition_image_layout(
	// 	    m_scene.m_framebuffer.m_resolve_texture->m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	// 	    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
	// 	    VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT);
	// 	command_buffer.transition_image_layout(
	// 	    m_framebuffer.m_color_texture->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	// 	    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
	// 	    VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);

	// 	VkImageCopy copy_info = {};
	// 	copy_info.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	// 	copy_info.srcSubresource.mipLevel = 0;
	// 	copy_info.srcSubresource.baseArrayLayer = 0;
	// 	copy_info.srcSubresource.layerCount = 1;
	// 	copy_info.srcOffset = { 0, 0, 0 };
	// 	copy_info.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	// 	copy_info.dstSubresource.mipLevel = 0;
	// 	copy_info.dstSubresource.baseArrayLayer = 0;
	// 	copy_info.dstSubresource.layerCount = 1;
	// 	copy_info.dstOffset = { (int)m_settings.viewport_x, (int)m_settings.viewport_y, 0 };
	// 	copy_info.extent = { m_settings.viewport_width, m_settings.viewport_height, 1 };
	// 	vkCmdCopyImage(command_buffer.m_handle, m_scene.m_framebuffer.m_resolve_texture->m_image.m_handle,
	// 	               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_framebuffer.m_color_texture->m_image.m_handle,
	// 	               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_info);

	// 	command_buffer.transition_image_layout(m_framebuffer.m_color_texture->m_image, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	// 	                                       VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
	// 	                                       VK_PIPELINE_STAGE_2_NONE, 0);

	// 	draw_ui(command_buffer);
	// }
	// m_context.end_frame(*m_framebuffer.m_color_texture);
}
