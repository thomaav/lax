#include <renderer/vulkan/render_pass.h>

#include "editor.h"

editor::~editor()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void editor::build_default()
{
	m_context.build();

	/* Settings derived from context (not really settings?). */
	m_settings.color_format = m_context.m_wsi.m_swapchain.m_images[0]->m_info.m_format;
	m_settings.depth_format = VK_FORMAT_D32_SFLOAT;

	/* Dear ImGui. */
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(m_context.m_window.m_window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_context.m_instance.m_handle;
	init_info.PhysicalDevice = m_context.m_device.m_physical.m_handle;
	init_info.Device = m_context.m_device.m_logical.m_handle;
	init_info.QueueFamily = *m_context.m_device.m_physical.m_queue_family.m_all;
	init_info.Queue = m_context.m_queue.m_handle;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPoolSize = 1024; /* Number of combined image samplers. */
	init_info.UseDynamicRendering = VK_TRUE;
	init_info.PipelineRenderingCreateInfo = {}; /* (TODO) */
	init_info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	init_info.PipelineRenderingCreateInfo.pNext = nullptr;
	init_info.PipelineRenderingCreateInfo.viewMask = 0;
	init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats =
	    &m_context.m_wsi.m_swapchain.m_images[0]->m_info.m_format;
	init_info.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
	init_info.PipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
	init_info.MinImageCount = m_context.m_wsi.m_swapchain.m_images.size();
	init_info.ImageCount = m_context.m_wsi.m_swapchain.m_images.size();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init_info);

	logger::register_logger(&m_logger);

	/* (TODO, thoave01): Settings, this is a throwaway render pass anyway. */
	vulkan::render_pass render_pass = {};
	render_pass.set_dynamic_rendering(true);
	render_pass.build(m_context.m_device, m_settings.color_format, m_settings.depth_format);

	m_scene.build_default_scene(m_context, render_pass);
}

void editor::draw(vulkan::command_buffer &command_buffer)
{
	/* (TODO, thoave01): Camera object. */
	m_scene.m_uniforms.view = m_scene.m_camera.m_view;
	m_scene.m_uniforms.projection = m_scene.m_camera.m_projection;
	m_scene.m_uniforms.enable_mipmapping = m_settings.enable_mipmapping;
	m_scene.m_uniform_buffer.fill(&m_scene.m_uniforms, sizeof(m_scene.m_uniforms));

	/* (TODO, thoave01): Add some sort of default pipeline with scene defaults and pipeline compatibility. */
	command_buffer.bind_pipeline(m_scene.m_skybox.m_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
	command_buffer.set_uniform_buffer(0, m_scene.m_uniform_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);

	for (auto &static_mesh : m_scene.m_static_meshes)
	{
		static_mesh->draw(command_buffer);
	}
	if (m_settings.enable_skybox)
	{
		m_scene.m_skybox.draw(command_buffer);
	}
}
