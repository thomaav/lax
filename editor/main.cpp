#include <renderer/vulkan/render_pass.h>
#include <renderer/vulkan/semaphore.h>

#include "editor.h"

int main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	editor editor = {};
	editor.build();

	vulkan::semaphore image_available_semaphore = {};
	image_available_semaphore.build(editor.m_context.m_device);
	vulkan::semaphore render_finished_semaphore = {};
	render_finished_semaphore.build(editor.m_context.m_device);

	vulkan::command_buffer command_buffer = {};
	command_buffer.build(editor.m_context.m_device, editor.m_context.m_command_pool);

	while (editor.m_context.m_window.step())
	{
		/* (TODO, thoave01): This doesn't belong in the context. But for now it has the platform. */
		camera_input camera_input = {};
		camera_input.w_pressed = editor.m_context.m_window.is_key_pressed(GLFW_KEY_W);
		camera_input.a_pressed = editor.m_context.m_window.is_key_pressed(GLFW_KEY_A);
		camera_input.s_pressed = editor.m_context.m_window.is_key_pressed(GLFW_KEY_S);
		camera_input.d_pressed = editor.m_context.m_window.is_key_pressed(GLFW_KEY_D);
		editor.m_context.m_window.get_mouse_position(camera_input.mouse_x, camera_input.mouse_y);
		camera_input.right_mouse_pressed = editor.m_context.m_window.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);
		camera_input.middle_mouse_pressed = editor.m_context.m_window.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_MIDDLE);
		editor.m_scene.m_camera.process_input(camera_input);

		vulkan::render_pass render_pass = {};
		render_pass.set_dynamic_rendering(true);
		render_pass.build(editor.m_context.m_device, editor.m_settings.color_format, editor.m_settings.depth_format);

		editor.m_ui.generate_frame();

		VkViewport viewport = {};
		viewport.x = editor.m_ui.m_viewport_x;
		viewport.y = (float)(editor.m_ui.m_viewport_y + editor.m_ui.m_viewport_height);
		viewport.width = (float)editor.m_ui.m_viewport_width;
		viewport.height = -(float)editor.m_ui.m_viewport_height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor = {};
		scissor.offset = { editor.m_ui.m_viewport_x, editor.m_ui.m_viewport_y };
		scissor.extent = { editor.m_ui.m_viewport_width, editor.m_ui.m_viewport_height };

		u32 image_idx = 0;
		editor.m_context.m_wsi.acquire_image(image_available_semaphore, &image_idx);

		editor.m_context.m_command_pool.reset();
		command_buffer.begin();
		{
			command_buffer.transition_image_layout(editor.m_scene.m_framebuffer.m_color_texture->m_image,
			                                       VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, 0,
			                                       VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0);
			command_buffer.transition_image_layout(editor.m_scene.m_framebuffer.m_resolve_texture->m_image,
			                                       VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, 0,
			                                       VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0);

			if (editor.m_settings.sample_count != VK_SAMPLE_COUNT_1_BIT)
			{
				VkClearValue clear_color = {};
				clear_color.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
				const VkRenderingAttachmentInfo color_attachment = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,                                      //
					.pNext = nullptr,                                                                          //
					.imageView = editor.m_scene.m_framebuffer.m_color_texture->m_image_view.m_handle,          //
					.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,                                   //
					.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT,                                                //
					.resolveImageView = editor.m_scene.m_framebuffer.m_resolve_texture->m_image_view.m_handle, //
					.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,                            //
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                                                     //
					.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,                                               //
					.clearValue = clear_color,                                                                 //
				};
				VkClearValue clear_depth = {};
				clear_depth.depthStencil = { 1.0f, 0 };
				const VkRenderingAttachmentInfo depth_attachment = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,                             //
					.pNext = nullptr,                                                                 //
					.imageView = editor.m_scene.m_framebuffer.m_depth_texture->m_image_view.m_handle, //
					.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,                          //
					.resolveMode = VK_RESOLVE_MODE_NONE,                                              //
					.resolveImageView = VK_NULL_HANDLE,                                               //
					.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,                                  //
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                                            //
					.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,                                      //
					.clearValue = clear_depth,                                                        //
				};
				const VkRenderingInfo rendering_info = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_INFO, //
					.pNext = nullptr,                          //
					.flags = 0,                                //
					.renderArea = { { 0, 0 },
					                { editor.m_scene.m_framebuffer.m_color_texture->m_image.m_info.m_width,
					                  editor.m_scene.m_framebuffer.m_color_texture->m_image.m_info.m_height } }, //
					.layerCount = 1,                                                                             //
					.viewMask = 0,                                                                               //
					.colorAttachmentCount = 1,                                                                   //
					.pColorAttachments = &color_attachment,                                                      //
					.pDepthAttachment = &depth_attachment,                                                       //
					.pStencilAttachment = nullptr,                                                               //
				};

				/* Render. */
				vkCmdBeginRendering(command_buffer.m_handle, &rendering_info);
				{
					vkCmdSetViewport(command_buffer.m_handle, 0, 1, &viewport);
					vkCmdSetScissor(command_buffer.m_handle, 0, 1, &scissor);
					editor.draw(command_buffer);
				}
				vkCmdEndRendering(command_buffer.m_handle);
			}
			else
			{
				VkClearValue clear_color = {};
				clear_color.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
				const VkRenderingAttachmentInfo color_attachment = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,                               //
					.pNext = nullptr,                                                                   //
					.imageView = editor.m_scene.m_framebuffer.m_resolve_texture->m_image_view.m_handle, //
					.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,                            //
					.resolveMode = VK_RESOLVE_MODE_NONE,                                                //
					.resolveImageView = VK_NULL_HANDLE,                                                 //
					.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,                                    //
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                                              //
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,                                            //
					.clearValue = clear_color,                                                          //
				};
				VkClearValue clear_depth = {};
				clear_depth.depthStencil = { 1.0f, 0 };
				const VkRenderingAttachmentInfo depth_attachment = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,                             //
					.pNext = nullptr,                                                                 //
					.imageView = editor.m_scene.m_framebuffer.m_depth_texture->m_image_view.m_handle, //
					.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,                          //
					.resolveMode = VK_RESOLVE_MODE_NONE,                                              //
					.resolveImageView = VK_NULL_HANDLE,                                               //
					.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,                                  //
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                                            //
					.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,                                      //
					.clearValue = clear_depth,                                                        //
				};
				const VkRenderingInfo rendering_info = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_INFO, //
					.pNext = nullptr,                          //
					.flags = 0,                                //
					.renderArea = { { 0, 0 },
					                { editor.m_scene.m_framebuffer.m_color_texture->m_image.m_info.m_width,
					                  editor.m_scene.m_framebuffer.m_color_texture->m_image.m_info.m_height } }, //
					.layerCount = 1,                                                                             //
					.viewMask = 0,                                                                               //
					.colorAttachmentCount = 1,                                                                   //
					.pColorAttachments = &color_attachment,                                                      //
					.pDepthAttachment = &depth_attachment,                                                       //
					.pStencilAttachment = nullptr,                                                               //
				};

				/* Render. */
				vkCmdBeginRendering(command_buffer.m_handle, &rendering_info);
				{
					vkCmdSetViewport(command_buffer.m_handle, 0, 1, &viewport);
					vkCmdSetScissor(command_buffer.m_handle, 0, 1, &scissor);
					editor.draw(command_buffer);
				}
				vkCmdEndRendering(command_buffer.m_handle);
			}

			command_buffer.transition_image_layout(
			    editor.m_scene.m_framebuffer.m_resolve_texture->m_image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			    VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

			const VkRenderingAttachmentInfo resolve_attachment = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,                               //
				.pNext = nullptr,                                                                   //
				.imageView = editor.m_scene.m_framebuffer.m_resolve_texture->m_image_view.m_handle, //
				.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,                            //
				.resolveMode = VK_RESOLVE_MODE_NONE,                                                //
				.resolveImageView = VK_NULL_HANDLE,                                                 //
				.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,                                    //
				.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,                                               //
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,                                            //
				.clearValue = {},                                                                   //
			};
			const VkRenderingInfo rendering_info_2 = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_INFO, //
				.pNext = nullptr,                          //
				.flags = 0,                                //
				.renderArea = { { 0, 0 },
				                { editor.m_scene.m_framebuffer.m_color_texture->m_image.m_info.m_width,
				                  editor.m_scene.m_framebuffer.m_color_texture->m_image.m_info.m_height } }, //
				.layerCount = 1,                                                                             //
				.viewMask = 0,                                                                               //
				.colorAttachmentCount = 1,                                                                   //
				.pColorAttachments = &resolve_attachment,                                                    //
				.pDepthAttachment = VK_NULL_HANDLE,                                                          //
				.pStencilAttachment = nullptr,                                                               //
			};
			vkCmdBeginRendering(command_buffer.m_handle, &rendering_info_2);
			{
				editor.m_ui.draw(command_buffer);
			}
			vkCmdEndRendering(command_buffer.m_handle);

			command_buffer.transition_image_layout(
			    editor.m_scene.m_framebuffer.m_resolve_texture->m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			    VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT);
			command_buffer.transition_image_layout(*editor.m_context.m_wsi.m_swapchain.m_images[image_idx],
			                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, 0,
			                                       VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);

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
			copy_info.dstOffset = { 0, 0, 0 };
			copy_info.extent = { editor.m_scene.m_framebuffer.m_color_texture->m_image.m_info.m_width,
				                 editor.m_scene.m_framebuffer.m_color_texture->m_image.m_info.m_height, 1 };
			vkCmdCopyImage(command_buffer.m_handle, editor.m_scene.m_framebuffer.m_resolve_texture->m_image.m_handle,
			               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			               editor.m_context.m_wsi.m_swapchain.m_images[image_idx]->m_handle,
			               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_info);

			command_buffer.transition_image_layout(*editor.m_context.m_wsi.m_swapchain.m_images[image_idx],
			                                       VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			                                       VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE, 0);
		}
		command_buffer.end();

		vulkan::fence unused_fence = {};
		vulkan::semaphore unused_semaphore = {};
		editor.m_context.m_queue.submit(command_buffer, image_available_semaphore, render_finished_semaphore,
		                                unused_fence);
		editor.m_context.m_queue.present(render_finished_semaphore, editor.m_context.m_wsi, image_idx);
		editor.m_context.m_device.wait();
	}
}
