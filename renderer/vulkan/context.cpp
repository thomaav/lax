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

struct uniforms
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	u32 enable_mipmapping;
};
static_assert(sizeof(uniforms) == 4 * 4 * 4 * 3 + sizeof(u32), "Unexpected struct uniform size");

constexpr u32 WINDOW_WIDTH = 1280;
constexpr u32 WINDOW_HEIGHT = 900;

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
	m_window.init(WINDOW_WIDTH, WINDOW_HEIGHT);
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
	editor.m_settings.enable_skybox = true;
	editor.m_settings.sample_count = VK_SAMPLE_COUNT_1_BIT;
	editor.m_settings.color_format = m_wsi.m_swapchain.m_images[0]->m_format;
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
	init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &m_wsi.m_swapchain.m_images[0]->m_format;
	init_info.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
	init_info.PipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
	init_info.MinImageCount = m_wsi.m_swapchain.m_images.size();
	init_info.ImageCount = m_wsi.m_swapchain.m_images.size();
	init_info.MSAASamples = VK_SAMPLE_COUNT_4_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init_info);

	/* (TODO, thoave01): So TODO is to fix uniforms for pipelines. */
	/* Uniform buffer. */
	uniforms uniforms = {};
	uniforms.model = editor.m_scene.m_static_mesh->m_model->m_meshes[0].m_transform;
	uniforms.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	uniforms.projection =
	    glm::perspectiveRH_ZO(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 256.0f);
	buffer uniform_buffer = {};
	m_resource_allocator.allocate_buffer(uniform_buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(uniforms));
	uniform_buffer.fill(&uniforms, sizeof(uniforms));

	/* Color texture. */
	image color_texture_image = {};
	color_texture_image.set_sample_count(VK_SAMPLE_COUNT_4_BIT);
	m_resource_allocator.allocate_image_2d(color_texture_image, editor.m_settings.color_format,
	                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
	                                           VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
	                                       m_wsi.m_swapchain.m_extent.width, m_wsi.m_swapchain.m_extent.height);
	color_texture_image.transition_layout(*this, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
	texture color_texture = {};
	color_texture.build(m_device, color_texture_image);

	/* Depth texture. */
	image depth_texture_image = {};
	depth_texture_image.set_sample_count(VK_SAMPLE_COUNT_4_BIT);
	m_resource_allocator.allocate_image_2d(depth_texture_image, VK_FORMAT_D32_SFLOAT,
	                                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	                                       m_wsi.m_swapchain.m_extent.width, m_wsi.m_swapchain.m_extent.height);
	depth_texture_image.transition_layout(*this, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	texture depth_texture = {};
	depth_texture.build(m_device, depth_texture_image);

	/* Resolve texture. */
	image resolve_texture_image = {};
	m_resource_allocator.allocate_image_2d(resolve_texture_image, editor.m_settings.color_format,
	                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	                                       m_wsi.m_swapchain.m_extent.width, m_wsi.m_swapchain.m_extent.height);
	resolve_texture_image.transition_layout(*this, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
	texture resolve_texture = {};
	resolve_texture.build(m_device, resolve_texture_image);

	for (std::unique_ptr<image> &image : m_wsi.m_swapchain.m_images)
	{
		image->transition_layout(*this, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	semaphore image_available_semaphore = {};
	image_available_semaphore.build(m_device);
	semaphore render_finished_semaphore = {};
	render_finished_semaphore.build(m_device);

	command_buffer command_buffer = {};
	command_buffer.build(m_device, m_command_pool);

	/* Settings. */
	std::vector<const char *> sample_counts = { "1xMSAA", "4xMSAA", "8xMSAA" };
	int sample_count_selection = 0;

	while (m_window.step())
	{
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
				case 2:
					editor.m_settings.sample_count = VK_SAMPLE_COUNT_8_BIT;
					break;
				}
			}
		}
		ImGui::End();
		ImGui::Render();

		/* Update uniforms. */
		static auto start_time = std::chrono::high_resolution_clock::now();
		auto current_time = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
		glm::mat4 time_rotation = glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		uniforms.model = time_rotation * editor.m_scene.m_static_mesh->m_model->m_meshes[0].m_transform;
		uniforms.enable_mipmapping = editor.m_settings.enable_mipmapping;
		uniform_buffer.fill(&uniforms, sizeof(uniforms));

		render_pass render_pass = {};
		render_pass.set_dynamic_rendering(true);
		render_pass.build(m_device, editor.m_settings.color_format, editor.m_settings.depth_format);

		editor.m_scene.m_skybox->update_material(*this, render_pass, VK_SAMPLE_COUNT_4_BIT);
		editor.m_scene.m_static_mesh->update_material(*this, render_pass, VK_SAMPLE_COUNT_4_BIT);

		u32 image_idx = 0;
		m_wsi.acquire_image(image_available_semaphore, &image_idx);

		m_command_pool.reset();
		command_buffer.begin();
		{
			command_buffer.transition_image_layout(*color_texture.m_image, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			                                       VK_PIPELINE_STAGE_2_NONE, 0,
			                                       VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0);
			command_buffer.transition_image_layout(*resolve_texture.m_image, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			                                       VK_PIPELINE_STAGE_2_NONE, 0,
			                                       VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0);

			/* Create render pass. */
			VkClearValue clear_color = {};
			clear_color.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
			const VkRenderingAttachmentInfo color_attachment = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,           //
				.pNext = nullptr,                                               //
				.imageView = color_texture.m_image_view.m_handle,               //
				.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,        //
				.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT,                     //
				.resolveImageView = resolve_texture.m_image_view.m_handle,      //
				.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, //
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                          //
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,                        //
				.clearValue = clear_color,                                      //
			};
			VkClearValue clear_depth = {};
			clear_depth.depthStencil = { 1.0f, 0 };
			const VkRenderingAttachmentInfo depth_attachment = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,    //
				.pNext = nullptr,                                        //
				.imageView = depth_texture.m_image_view.m_handle,        //
				.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, //
				.resolveMode = VK_RESOLVE_MODE_NONE,                     //
				.resolveImageView = VK_NULL_HANDLE,                      //
				.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,         //
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                   //
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,             //
				.clearValue = clear_depth,                               //
			};
			const VkRenderingInfo rendering_info = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,                                                       //
				.pNext = nullptr,                                                                                //
				.flags = 0,                                                                                      //
				.renderArea = { { 0, 0 }, { color_texture.m_image->m_width, color_texture.m_image->m_height } }, //
				.layerCount = 1,                                                                                 //
				.viewMask = 0,                                                                                   //
				.colorAttachmentCount = 1,                                                                       //
				.pColorAttachments = &color_attachment,                                                          //
				.pDepthAttachment = &depth_attachment,                                                           //
				.pStencilAttachment = nullptr,                                                                   //
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

				editor.m_scene.m_root.m_children[0].m_object->draw(command_buffer, uniform_buffer);
				if (editor.m_settings.enable_skybox)
				{
					editor.m_scene.m_root.m_children[1].m_object->draw(command_buffer, uniform_buffer);
				}

				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer.m_handle);
			}
			vkCmdEndRendering(command_buffer.m_handle);

			command_buffer.transition_image_layout(*resolve_texture.m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
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
			copy_info.extent = { color_texture.m_image->m_width, color_texture.m_image->m_height, 1 };
			vkCmdCopyImage(command_buffer.m_handle, resolve_texture.m_image->m_handle,
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
