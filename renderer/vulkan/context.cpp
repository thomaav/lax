#include <algorithm>
#include <chrono>
#include <iostream>
#include <optional>
#include <string_view>

// clang-format off
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <third_party/volk/volk.h>
#include <third_party/imgui/imgui.h>
#include <third_party/imgui/imgui_impl_glfw.h>
#include <third_party/imgui/imgui_impl_vulkan.h>
#pragma clang diagnostic pop
// clang-format on

#include <assets/image.h>
#include <assets/model.h>
#include <platform/window.h>
#include <renderer/editor.h>
#include <renderer/scene.h>
#include <renderer/vulkan/buffer.h>
#include <renderer/vulkan/command_buffer.h>
#include <renderer/vulkan/context.h>
#include <renderer/vulkan/fence.h>
#include <renderer/vulkan/pipeline.h>
#include <renderer/vulkan/render_pass.h>
#include <renderer/vulkan/semaphore.h>
#include <renderer/vulkan/shader.h>
#include <renderer/vulkan/util.h>
#include <utils/type.h>
#include <utils/util.h>

namespace vulkan
{

context::~context()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void context::add_instance_extension(const char *extension)
{
	m_instance.add_extension(extension);
}

void context::add_device_extension(const char *extension)
{
	m_device.add_extension(extension);
}

void context::build()
{
	/* Initialize loading. */
	VULKAN_ASSERT_SUCCESS(volkInitialize());

	m_device.add_extension("VK_KHR_push_descriptor");
	const VpProfileProperties profile_properties = {
		VP_LUNARG_MINIMUM_REQUIREMENTS_1_3_NAME,        //
		VP_LUNARG_MINIMUM_REQUIREMENTS_1_3_SPEC_VERSION //
	};

	/* Instance initialization. */
	m_window.init();
	m_instance.build(m_window, profile_properties);
	volkLoadInstance(m_instance.m_handle);
	m_wsi.build_surface(m_window, m_instance);

	/* Device initialization. */
	m_device.build(m_instance, m_wsi.m_surface.handle, profile_properties);
	volkLoadDevice(m_device.m_logical.m_handle);
	m_wsi.build_swapchain(m_device);
	m_queue.build(m_device);

	/* Resource management initialization. */
	m_resource_allocator.build(m_instance, m_device);
	m_command_pool.build(m_device);
}

void context::backend_test()
{
	editor editor = {};
	editor.m_settings.enable_mipmapping = true;
	editor.m_settings.enable_skybox = false;
	editor.m_settings.sample_count = VK_SAMPLE_COUNT_4_BIT;
	editor.m_settings.color_format = m_wsi.m_swapchain.m_images[0]->m_info.m_format;
	editor.m_settings.depth_format = VK_FORMAT_D32_SFLOAT;
	editor.build_default(*this);

	/* Dear ImGui. */
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	UNUSED(io);

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(m_window.m_window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_instance.m_handle;
	init_info.PhysicalDevice = m_device.m_physical.m_handle;
	init_info.Device = m_device.m_logical.m_handle;
	init_info.QueueFamily = *m_device.m_physical.m_queue_family.m_all;
	init_info.Queue = m_queue.m_handle;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPoolSize = 1024; /* Number of combined image samplers. */
	init_info.UseDynamicRendering = VK_TRUE;
	init_info.PipelineRenderingCreateInfo = {}; /* (TODO) */
	init_info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	init_info.PipelineRenderingCreateInfo.pNext = nullptr;
	init_info.PipelineRenderingCreateInfo.viewMask = 0;
	init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &m_wsi.m_swapchain.m_images[0]->m_info.m_format;
	init_info.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
	init_info.PipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
	init_info.MinImageCount = m_wsi.m_swapchain.m_images.size();
	init_info.ImageCount = m_wsi.m_swapchain.m_images.size();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init_info);

	/* Color texture. */
	ref<texture> color_texture = make_ref<texture>();
	color_texture->build(*this,
	                     { .m_format = editor.m_settings.color_format,
	                       .m_width = m_wsi.m_swapchain.m_extent.width,
	                       .m_height = m_wsi.m_swapchain.m_extent.height,
	                       .m_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
	                       .m_sample_count = editor.m_settings.sample_count });

	/* Depth texture. */
	ref<texture> depth_texture = make_ref<texture>();
	depth_texture->build(*this, { .m_format = VK_FORMAT_D32_SFLOAT,
	                              .m_width = m_wsi.m_swapchain.m_extent.width,
	                              .m_height = m_wsi.m_swapchain.m_extent.height,
	                              .m_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	                              .m_sample_count = editor.m_settings.sample_count });

	/* Resolve texture. */
	ref<texture> resolve_texture = make_ref<texture>();
	resolve_texture->build(*this, { .m_format = editor.m_settings.color_format,
	                                .m_width = m_wsi.m_swapchain.m_extent.width,
	                                .m_height = m_wsi.m_swapchain.m_extent.height,
	                                .m_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT });

	semaphore image_available_semaphore = {};
	image_available_semaphore.build(m_device);
	semaphore render_finished_semaphore = {};
	render_finished_semaphore.build(m_device);

	command_buffer command_buffer = {};
	command_buffer.build(m_device, m_command_pool);

	/* Settings. */
	std::vector<const char *> sample_counts = { "1xMSAA", "4xMSAA" };
	int sample_count_selection = 1;

	/* (TODO, thoave01): This should be part of initializing the defaults. */
	render_pass render_pass_ = {};
	render_pass_.set_dynamic_rendering(true);
	render_pass_.build(m_device, editor.m_settings.color_format, editor.m_settings.depth_format);
	editor.m_scene.m_skybox->update_material(render_pass_, editor.m_settings.sample_count);
	editor.m_scene.m_static_mesh->update_material(render_pass_, editor.m_settings.sample_count);

	while (m_window.step())
	{
		/* (TODO, thoave01): This doesn't belong in the context. But for now it has the platform. */
		camera_input camera_input = {};
		camera_input.w_pressed = m_window.is_key_pressed(GLFW_KEY_W);
		camera_input.a_pressed = m_window.is_key_pressed(GLFW_KEY_A);
		camera_input.s_pressed = m_window.is_key_pressed(GLFW_KEY_S);
		camera_input.d_pressed = m_window.is_key_pressed(GLFW_KEY_D);
		m_window.get_mouse_position(camera_input.mouse_x, camera_input.mouse_y);
		camera_input.right_mouse_pressed = m_window.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);
		camera_input.middle_mouse_pressed = m_window.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_MIDDLE);
		editor.m_scene.m_camera.process_input(camera_input);

		render_pass render_pass = {};
		render_pass.set_dynamic_rendering(true);
		render_pass.build(m_device, editor.m_settings.color_format, editor.m_settings.depth_format);

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Lax", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		{
			ImGui::Checkbox("Skybox", &editor.m_settings.enable_skybox);
			ImGui::Checkbox("Mipmapping", &editor.m_settings.enable_mipmapping);
			if (ImGui::Combo("##MSAA", &sample_count_selection, sample_counts.data(), sample_counts.size()))
			{
				switch (sample_count_selection)
				{
				case 0:
					editor.m_settings.sample_count = VK_SAMPLE_COUNT_1_BIT;
					break;
				case 1:
					editor.m_settings.sample_count = VK_SAMPLE_COUNT_4_BIT;
					break;
				}

				editor.m_scene.m_skybox->update_material(render_pass, editor.m_settings.sample_count);
				editor.m_scene.m_static_mesh->update_material(render_pass, editor.m_settings.sample_count);

				color_texture = make_ref<texture>();
				color_texture->build(
				    *this, { .m_format = editor.m_settings.color_format,
				             .m_width = m_wsi.m_swapchain.m_extent.width,
				             .m_height = m_wsi.m_swapchain.m_extent.height,
				             .m_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
				             .m_sample_count = editor.m_settings.sample_count });

				/* Depth texture. */
				depth_texture = make_ref<texture>();
				depth_texture->build(*this, { .m_format = VK_FORMAT_D32_SFLOAT,
				                              .m_width = m_wsi.m_swapchain.m_extent.width,
				                              .m_height = m_wsi.m_swapchain.m_extent.height,
				                              .m_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				                              .m_sample_count = editor.m_settings.sample_count });
			}
		}
		ImGui::End();
		ImGui::Render();

		u32 image_idx = 0;
		m_wsi.acquire_image(image_available_semaphore, &image_idx);

		m_command_pool.reset();
		command_buffer.begin();
		{
			command_buffer.transition_image_layout(color_texture->m_image, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			                                       VK_PIPELINE_STAGE_2_NONE, 0,
			                                       VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0);
			command_buffer.transition_image_layout(resolve_texture->m_image, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			                                       VK_PIPELINE_STAGE_2_NONE, 0,
			                                       VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0);

			if (editor.m_settings.sample_count != VK_SAMPLE_COUNT_1_BIT)
			{
				VkClearValue clear_color = {};
				clear_color.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
				const VkRenderingAttachmentInfo color_attachment = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,           //
					.pNext = nullptr,                                               //
					.imageView = color_texture->m_image_view.m_handle,              //
					.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,        //
					.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT,                     //
					.resolveImageView = resolve_texture->m_image_view.m_handle,     //
					.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, //
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                          //
					.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,                    //
					.clearValue = clear_color,                                      //
				};
				VkClearValue clear_depth = {};
				clear_depth.depthStencil = { 1.0f, 0 };
				const VkRenderingAttachmentInfo depth_attachment = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,    //
					.pNext = nullptr,                                        //
					.imageView = depth_texture->m_image_view.m_handle,       //
					.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, //
					.resolveMode = VK_RESOLVE_MODE_NONE,                     //
					.resolveImageView = VK_NULL_HANDLE,                      //
					.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,         //
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                   //
					.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,             //
					.clearValue = clear_depth,                               //
				};
				const VkRenderingInfo rendering_info = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_INFO, //
					.pNext = nullptr,                          //
					.flags = 0,                                //
					.renderArea = { { 0, 0 },
					                { color_texture->m_image.m_info.m_width,
					                  color_texture->m_image.m_info.m_height } }, //
					.layerCount = 1,                                              //
					.viewMask = 0,                                                //
					.colorAttachmentCount = 1,                                    //
					.pColorAttachments = &color_attachment,                       //
					.pDepthAttachment = &depth_attachment,                        //
					.pStencilAttachment = nullptr,                                //
				};

				/* Render. */
				vkCmdBeginRendering(command_buffer.m_handle, &rendering_info);
				{
					VkViewport viewport = {};
					viewport.x = 0.0f;
					/* (TODO, thoave01): Not WSI, but color image. */
					viewport.y = (float)m_wsi.m_swapchain.m_extent.height;
					viewport.width = (float)m_wsi.m_swapchain.m_extent.width;
					viewport.height = -(float)m_wsi.m_swapchain.m_extent.height;
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;
					vkCmdSetViewport(command_buffer.m_handle, 0, 1, &viewport);
					VkRect2D scissor = {};
					scissor.offset = { 0, 0 };
					scissor.extent = m_wsi.m_swapchain.m_extent;
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
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,    //
					.pNext = nullptr,                                        //
					.imageView = resolve_texture->m_image_view.m_handle,     //
					.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, //
					.resolveMode = VK_RESOLVE_MODE_NONE,                     //
					.resolveImageView = VK_NULL_HANDLE,                      //
					.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,         //
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                   //
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,                 //
					.clearValue = clear_color,                               //
				};
				VkClearValue clear_depth = {};
				clear_depth.depthStencil = { 1.0f, 0 };
				const VkRenderingAttachmentInfo depth_attachment = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,    //
					.pNext = nullptr,                                        //
					.imageView = depth_texture->m_image_view.m_handle,       //
					.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, //
					.resolveMode = VK_RESOLVE_MODE_NONE,                     //
					.resolveImageView = VK_NULL_HANDLE,                      //
					.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,         //
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                   //
					.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,             //
					.clearValue = clear_depth,                               //
				};
				const VkRenderingInfo rendering_info = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_INFO, //
					.pNext = nullptr,                          //
					.flags = 0,                                //
					.renderArea = { { 0, 0 },
					                { color_texture->m_image.m_info.m_width,
					                  color_texture->m_image.m_info.m_height } }, //
					.layerCount = 1,                                              //
					.viewMask = 0,                                                //
					.colorAttachmentCount = 1,                                    //
					.pColorAttachments = &color_attachment,                       //
					.pDepthAttachment = &depth_attachment,                        //
					.pStencilAttachment = nullptr,                                //
				};

				/* Render. */
				vkCmdBeginRendering(command_buffer.m_handle, &rendering_info);
				{
					VkViewport viewport = {};
					viewport.x = 0.0f;
					/* (TODO, thoave01): Not WSI, but color image. */
					viewport.y = (float)m_wsi.m_swapchain.m_extent.height;
					viewport.width = (float)m_wsi.m_swapchain.m_extent.width;
					viewport.height = -(float)m_wsi.m_swapchain.m_extent.height;
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;
					vkCmdSetViewport(command_buffer.m_handle, 0, 1, &viewport);
					VkRect2D scissor = {};
					scissor.offset = { 0, 0 };
					scissor.extent = m_wsi.m_swapchain.m_extent;
					vkCmdSetScissor(command_buffer.m_handle, 0, 1, &scissor);

					editor.draw(command_buffer);
				}
				vkCmdEndRendering(command_buffer.m_handle);
			}

			command_buffer.transition_image_layout(
			    resolve_texture->m_image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			    VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

			const VkRenderingAttachmentInfo resolve_attachment = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,    //
				.pNext = nullptr,                                        //
				.imageView = resolve_texture->m_image_view.m_handle,     //
				.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, //
				.resolveMode = VK_RESOLVE_MODE_NONE,                     //
				.resolveImageView = VK_NULL_HANDLE,                      //
				.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,         //
				.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,                    //
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,                 //
				.clearValue = {},                                        //
			};
			const VkRenderingInfo rendering_info_2 = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_INFO, //
				.pNext = nullptr,                          //
				.flags = 0,                                //
				.renderArea = { { 0, 0 },
				                { color_texture->m_image.m_info.m_width, color_texture->m_image.m_info.m_height } }, //
				.layerCount = 1,                                                                                     //
				.viewMask = 0,                                                                                       //
				.colorAttachmentCount = 1,                                                                           //
				.pColorAttachments = &resolve_attachment,                                                            //
				.pDepthAttachment = VK_NULL_HANDLE,                                                                  //
				.pStencilAttachment = nullptr,                                                                       //
			};
			vkCmdBeginRendering(command_buffer.m_handle, &rendering_info_2);
			{
				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer.m_handle);
			}
			vkCmdEndRendering(command_buffer.m_handle);

			command_buffer.transition_image_layout(resolve_texture->m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			                                       VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			                                       VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			                                       VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT);
			command_buffer.transition_image_layout(*m_wsi.m_swapchain.m_images[image_idx],
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
			copy_info.extent = { color_texture->m_image.m_info.m_width, color_texture->m_image.m_info.m_height, 1 };
			vkCmdCopyImage(command_buffer.m_handle, resolve_texture->m_image.m_handle,
			               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_wsi.m_swapchain.m_images[image_idx]->m_handle,
			               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_info);

			command_buffer.transition_image_layout(*m_wsi.m_swapchain.m_images[image_idx],
			                                       VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			                                       VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE, 0);
		}
		command_buffer.end();

		fence unused_fence = {};
		semaphore unused_semaphore = {};
		m_queue.submit(command_buffer, image_available_semaphore, render_finished_semaphore, unused_fence);
		m_queue.present(render_finished_semaphore, m_wsi, image_idx);
		m_device.wait();
	}
}

} /* namespace vulkan */
