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
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init_info);
}

void context::backend_test()
{
	render_pass render_pass = {};
	render_pass.set_dynamic_rendering(true);
	render_pass.build(m_device, m_wsi.m_swapchain.m_images[0]->m_format, VK_FORMAT_D32_SFLOAT);

	editor editor = {};
	editor.m_scene.build_default_scene(*this, render_pass);

	/* (TODO, thoave01): Should be in build_default_scene, but I need the transform. */
	/* (TODO, thoave01): So TODO is to fix uniforms for the pipeline. */
	ref<assets::model> model = make_ref<assets::model>();
	model->load("bin/assets/models/DamagedHelmet.glb");

	/* Uniform buffer. */
	uniforms uniforms = {};
	uniforms.model = model->m_meshes[0].m_transform;
	uniforms.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	uniforms.projection =
	    glm::perspectiveRH_ZO(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 256.0f);
	buffer uniform_buffer = {};
	m_resource_allocator.allocate_buffer(uniform_buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(uniforms));
	uniform_buffer.fill(&uniforms, sizeof(uniforms));

	/* Depth texture. */
	image depth_texture_image = {};
	m_resource_allocator.allocate_image_2d(depth_texture_image, VK_FORMAT_D32_SFLOAT,
	                                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	                                       m_wsi.m_swapchain.m_extent.width, m_wsi.m_swapchain.m_extent.height);
	depth_texture_image.transition_layout(*this, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	texture depth_texture = {};
	depth_texture.build(m_device, depth_texture_image);

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

	settings settings = {
		.enable_mipmapping = true,            //
		.enable_skybox = true,                //
		.sample_count = VK_SAMPLE_COUNT_1_BIT //
	};

	scene main_scene = {};
	static_mesh static_mesh = {};
	static_mesh.build(*this, render_pass, model);
	skybox skybox = {};
	skybox.build(*this, render_pass);

	while (m_window.step())
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Lax", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		{
			ImGui::Checkbox("Skybox", &settings.enable_skybox);
			ImGui::Checkbox("Mipmapping", &settings.enable_mipmapping);
			if (ImGui::Combo("##MSAA", &sample_count_selection, sample_counts.data(), sample_counts.size()))
			{
				switch (sample_count_selection)
				{
				case 0:
					settings.sample_count = VK_SAMPLE_COUNT_1_BIT;
					break;
				case 1:
					settings.sample_count = VK_SAMPLE_COUNT_4_BIT;
					break;
				case 2:
					settings.sample_count = VK_SAMPLE_COUNT_8_BIT;
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
		uniforms.model = time_rotation * model->m_meshes[0].m_transform;
		uniforms.enable_mipmapping = settings.enable_mipmapping;
		uniform_buffer.fill(&uniforms, sizeof(uniforms));

		u32 image_idx = 0;
		m_wsi.acquire_image(image_available_semaphore, &image_idx);

		m_command_pool.reset();
		command_buffer.begin();
		{
			/* Transition image for rendering. */
			const VkImageSubresourceRange img_range = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, //
				.baseMipLevel = 0,                       //
				.levelCount = 1,                         //
				.baseArrayLayer = 0,                     //
				.layerCount = 1,                         //
			};
			const VkImageMemoryBarrier2 begin_rendering_barrier = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,               //
				.pNext = nullptr,                                                //
				.srcStageMask = VK_PIPELINE_STAGE_2_NONE,                        //
				.srcAccessMask = 0,                                              //
				.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, //
				.dstAccessMask = 0,                                              //
				.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                    //
				.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,                 //
				.srcQueueFamilyIndex = 0,                                        //
				.dstQueueFamilyIndex = 0,                                        //
				.image = m_wsi.m_swapchain.m_vulkan_images[image_idx],           //
				.subresourceRange = img_range,                                   //
			};
			const VkDependencyInfo begin_rendering_dependency_info = {
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,       //
				.pNext = nullptr,                                 //
				.dependencyFlags = 0,                             //
				.memoryBarrierCount = 0,                          //
				.pMemoryBarriers = nullptr,                       //
				.bufferMemoryBarrierCount = 0,                    //
				.pBufferMemoryBarriers = nullptr,                 //
				.imageMemoryBarrierCount = 1,                     //
				.pImageMemoryBarriers = &begin_rendering_barrier, //
			};
			vkCmdPipelineBarrier2(command_buffer.m_handle, &begin_rendering_dependency_info);

			/* Create render pass. */
			VkClearValue clear_color = {};
			clear_color.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
			const VkRenderingAttachmentInfo color_attachment = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,              //
				.pNext = nullptr,                                                  //
				.imageView = m_wsi.m_swapchain.m_image_views[image_idx]->m_handle, //
				.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,           //
				.resolveMode = VK_RESOLVE_MODE_NONE,                               //
				.resolveImageView = VK_NULL_HANDLE,                                //
				.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,                   //
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                             //
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,                           //
				.clearValue = clear_color,                                         //
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
				.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,              //
				.pNext = nullptr,                                       //
				.flags = 0,                                             //
				.renderArea = { { 0, 0 }, m_wsi.m_swapchain.m_extent }, //
				.layerCount = 1,                                        //
				.viewMask = 0,                                          //
				.colorAttachmentCount = 1,                              //
				.pColorAttachments = &color_attachment,                 //
				.pDepthAttachment = &depth_attachment,                  //
				.pStencilAttachment = nullptr,                          //
			};

			/* Render. */
			vkCmdBeginRendering(command_buffer.m_handle, &rendering_info);
			{
				VkViewport viewport = {};
				viewport.x = 0.0f;
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

				static_mesh.draw(command_buffer, uniform_buffer);
				if (settings.enable_skybox)
				{
					skybox.draw(command_buffer, uniform_buffer);
				}

				editor.m_scene.m_root.m_children[0].m_object->draw(command_buffer, uniform_buffer);
				editor.m_scene.m_root.m_children[1].m_object->draw(command_buffer, uniform_buffer);

				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer.m_handle);
			}
			vkCmdEndRendering(command_buffer.m_handle);

			/* Transition image for presentation. */
			VkImageMemoryBarrier2 end_rendering_barrier = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,               //
				.pNext = nullptr,                                                //
				.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, //
				.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,         //
				.dstStageMask = VK_PIPELINE_STAGE_2_NONE,                        //
				.dstAccessMask = 0,                                              //
				.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,                 //
				.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                    //
				.srcQueueFamilyIndex = 0,                                        //
				.dstQueueFamilyIndex = 0,                                        //
				.image = m_wsi.m_swapchain.m_vulkan_images[image_idx],           //
				.subresourceRange = img_range,                                   //
			};
			VkDependencyInfo end_rendering_dependency_info = {
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,     //
				.pNext = nullptr,                               //
				.dependencyFlags = 0,                           //
				.memoryBarrierCount = 0,                        //
				.pMemoryBarriers = nullptr,                     //
				.bufferMemoryBarrierCount = 0,                  //
				.pBufferMemoryBarriers = nullptr,               //
				.imageMemoryBarrierCount = 1,                   //
				.pImageMemoryBarriers = &end_rendering_barrier, //
			};
			vkCmdPipelineBarrier2(command_buffer.m_handle, &end_rendering_dependency_info);
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
