#include <renderer/vulkan/render_pass.h>
#include <renderer/vulkan/semaphore.h>

#include "editor.h"

int main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	editor editor = {};
	editor.build();

	while (editor.m_context.m_window.step())
	{
		editor.update();

		vulkan::command_buffer &command_buffer = editor.m_context.begin_frame();
		{
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
		}
		editor.m_context.end_frame(*editor.m_scene.m_framebuffer.m_resolve_texture);
	}
}
