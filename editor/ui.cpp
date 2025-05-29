#include <chrono>

// clang-format off
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#include <third_party/imgui/imgui.h>
#include <third_party/imgui/imgui_internal.h>v
#include <third_party/imgui/imgui_impl_glfw.h>
#include <third_party/imgui/imgui_impl_vulkan.h>
#pragma clang diagnostic pop
// clang-format on

#include <renderer/vulkan/command_buffer.h>

#include "editor.h"
#include "ui.h"

ui::~ui()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ui::build(editor &editor)
{
	m_editor = &editor;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(m_editor->m_context.m_window.m_window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_editor->m_context.m_instance.m_handle;
	init_info.PhysicalDevice = m_editor->m_context.m_device.m_physical.m_handle;
	init_info.Device = m_editor->m_context.m_device.m_logical.m_handle;
	init_info.QueueFamily = *m_editor->m_context.m_device.m_physical.m_queue_family.m_all;
	init_info.Queue = m_editor->m_context.m_queue.m_handle;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPoolSize = 1024; /* Number of combined image samplers. */
	init_info.UseDynamicRendering = VK_TRUE;
	init_info.PipelineRenderingCreateInfo = {};
	init_info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	init_info.PipelineRenderingCreateInfo.pNext = nullptr;
	init_info.PipelineRenderingCreateInfo.viewMask = 0;
	init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats =
	    &m_editor->m_context.m_wsi.m_swapchain.m_images[0]->m_info.m_format;
	init_info.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
	init_info.PipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
	init_info.MinImageCount = m_editor->m_context.m_wsi.m_swapchain.m_images.size();
	init_info.ImageCount = m_editor->m_context.m_wsi.m_swapchain.m_images.size();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init_info);
}

void ui::generate_frame()
{
	assert_if(nullptr == m_editor, "UI has no editor reference");

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	/* Setup. */
	generate_docking();
	ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_None;

	/* Generate UI. */
	generate_console();
	generate_scene();
	generate_settings();
	generate_debug();
	generate_viewport();
	ImGui::Render();
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}

void ui::draw(vulkan::command_buffer &command_buffer)
{
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer.m_handle);
}

void ui::generate_docking()
{
	static bool first_frame = true;
	if (first_frame)
	{
		ImGuiID window_id = ImGui::DockSpaceOverViewport();

		ImGui::DockBuilderRemoveNode(window_id);
		ImGui::DockBuilderAddNode(window_id, ImGuiDockNodeFlags_None);
		ImGui::DockBuilderSetNodeSize(window_id, ImGui::GetMainViewport()->Size);

		ImGuiID dock_bottom_id = ImGui::DockBuilderSplitNode(window_id, ImGuiDir_Down, 0.15f, nullptr, &window_id);
		ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(window_id, ImGuiDir_Left, 0.15f, nullptr, &window_id);
		ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(window_id, ImGuiDir_Right, 0.15f, nullptr, &window_id);
		ImGuiID dock_right_bottom_id =
		    ImGui::DockBuilderSplitNode(dock_right_id, ImGuiDir_Down, 0.60f, nullptr, &dock_right_id);

		ImGui::DockBuilderDockWindow("Console", dock_bottom_id);
		ImGui::DockBuilderDockWindow("Scene", dock_left_id);
		ImGui::DockBuilderDockWindow("Settings", dock_right_id);
		ImGui::DockBuilderDockWindow("Debug", dock_right_bottom_id);
		ImGui::DockBuilderDockWindow("Viewport", window_id);

		ImGui::DockBuilderFinish(window_id);

		first_frame = false;
	}
}

void ui::generate_console()
{
	ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
	{
		ImGui::PushTextWrapPos();
		ImGui::TextUnformatted(m_editor->m_logger.m_buffer.begin(), m_editor->m_logger.m_buffer.end());
		ImGui::PopTextWrapPos();
		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		{
			ImGui::SetScrollHereY(1.0f);
		}
	}
	ImGui::End();
}

void ui::generate_scene()
{
	ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoMove);
	{
		if (ImGui::TreeNodeEx("Root", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::TreeNodeEx("Static meshes", ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (auto &[e, static_mesh] : m_editor->m_scene.m_static_mesh_storage)
				{
					ImGui::Text("[e%lu] Static mesh", e);
				}
				ImGui::TreePop();
			}
			for (auto &[e, skybox] : m_editor->m_scene.m_skybox_storage)
			{
				ImGui::Text("[e%lu] Skybox", e);
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();
}

void ui::generate_settings()
{
	static std::vector<const char *> sample_counts = { "1xMSAA", "4xMSAA" };
	static int sample_count_selection = 1;
	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
	{
		ImGui::Checkbox("Skybox", &m_editor->m_settings.enable_skybox);
		ImGui::Checkbox("Mipmapping", &m_editor->m_settings.enable_mipmapping);
		ImGui::Checkbox("Enable grid", &m_editor->m_settings.enable_grid);
		if (ImGui::Combo("##MSAA", &sample_count_selection, sample_counts.data(), sample_counts.size()))
		{
			switch (sample_count_selection)
			{
			case 0:
				m_editor->m_settings.sample_count = VK_SAMPLE_COUNT_1_BIT;
				break;
			case 1:
				m_editor->m_settings.sample_count = VK_SAMPLE_COUNT_4_BIT;
				break;
			}
		}
	}
	ImGui::End();
}

void ui::generate_debug()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoMove);
	{
		static bool first = true;
		static auto last = std::chrono::steady_clock::now();
		static std::vector<float> frame_times = {};

		auto now = std::chrono::steady_clock::now();
		float ms = std::chrono::duration<float, std::ratio<1, 1000>>(now - last).count();
		last = now;

		if (!first)
		{
			frame_times.push_back(ms);
			if (frame_times.size() > 100)
			{
				frame_times.erase(frame_times.begin());
			}
			auto [min, max] = std::minmax_element(frame_times.begin(), frame_times.end());
			ImGui::PlotLines("##frame_time", frame_times.data(), frame_times.size(), 0, "Frame time", 0.0, 33.3,
			                 ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight() * 0.2f));
		}
		else
		{
			first = false;
		}

		float window_width = ImGui::GetWindowSize().x / 2.0f;
		float ms_text_width = ImGui::CalcTextSize(" MS %.1f").x;
		ImGui::SetCursorPosX((window_width - ms_text_width) * 0.5f);
		ImGui::Text(" MS %.1f", ms);

		ImGui::SameLine(ImGui::GetWindowWidth() / 2.0f);
		float fps_text_width = ImGui::CalcTextSize("FPS %.1f").x;
		ImGui::SetCursorPosX(window_width + (window_width - fps_text_width) * 0.5f);
		ImGui::Text("FPS %.1f", ImGui::GetIO().Framerate);
	}
	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void ui::generate_viewport()
{
	ImGui::Begin("Viewport", nullptr,
	             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
	{
		ImGui::GetWindowDockNode()->SetLocalFlags(ImGuiDockNodeFlags_NoTabBar);
		m_editor->m_settings.viewport_x = ImGui::GetWindowPos().x;
		m_editor->m_settings.viewport_y = ImGui::GetWindowPos().y;
		m_editor->m_settings.viewport_width = ImGui::GetWindowWidth();
		m_editor->m_settings.viewport_height = ImGui::GetWindowHeight();
	}
	ImGui::End();
}
