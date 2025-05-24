// clang-format off
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#include <third_party/imgui/imgui.h>
#include <third_party/imgui/imgui_impl_glfw.h>
#include <third_party/imgui/imgui_impl_vulkan.h>
#pragma clang diagnostic pop
// clang-format on

#include "ui.h"

ui::~ui()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ui::build(vulkan::context &context)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(context.m_window.m_window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = context.m_instance.m_handle;
	init_info.PhysicalDevice = context.m_device.m_physical.m_handle;
	init_info.Device = context.m_device.m_logical.m_handle;
	init_info.QueueFamily = *context.m_device.m_physical.m_queue_family.m_all;
	init_info.Queue = context.m_queue.m_handle;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPoolSize = 1024; /* Number of combined image samplers. */
	init_info.UseDynamicRendering = VK_TRUE;
	init_info.PipelineRenderingCreateInfo = {}; /* (TODO) */
	init_info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	init_info.PipelineRenderingCreateInfo.pNext = nullptr;
	init_info.PipelineRenderingCreateInfo.viewMask = 0;
	init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats =
	    &context.m_wsi.m_swapchain.m_images[0]->m_info.m_format;
	init_info.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
	init_info.PipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
	init_info.MinImageCount = context.m_wsi.m_swapchain.m_images.size();
	init_info.ImageCount = context.m_wsi.m_swapchain.m_images.size();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init_info);
}
