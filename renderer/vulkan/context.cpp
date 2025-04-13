#include <algorithm>
#include <iostream>
#include <optional>
#include <string_view>

#include <third_party/volk/volk.h>
#include <vulkan/vulkan_profiles.hpp>

#include <platform/window.h>
#include <renderer/vulkan/command_buffer.h>
#include <renderer/vulkan/command_pool.h>
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
	/* Destroy device. */
	m_wsi.destroy_swapchain();
	m_device.destroy();

	/* Destroy instance. */
	m_wsi.destroy_surface();
	m_instance.destroy();
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

	const VpProfileProperties profile_properties = {
		VP_LUNARG_MINIMUM_REQUIREMENTS_1_3_NAME,        //
		VP_LUNARG_MINIMUM_REQUIREMENTS_1_3_SPEC_VERSION //
	};

	/* Instance initialization. */
	m_window.init(800, 600);
	m_instance.build(m_window, profile_properties);
	volkLoadInstance(m_instance.m_handle);
	m_wsi.build_surface(m_window, m_instance);

	/* Device initialization. */
	m_device.build(m_instance, m_wsi.m_surface.handle, profile_properties);
	volkLoadDevice(m_device.m_logical.m_handle);
	m_wsi.build_swapchain(m_device);
	m_queue.build(m_device);
}

void context::backend_test()
{
	shader_module vertex_shader_module = {};
	vertex_shader_module.build(m_device, VK_SHADER_STAGE_VERTEX_BIT, "bin/shaders/basic.vert.spv");

	shader_module fragment_shader_module = {};
	fragment_shader_module.build(m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "bin/shaders/basic.frag.spv");

	pipeline_layout pipeline_layout = {};
	pipeline_layout.build(m_device);

	render_pass render_pass = {};
	render_pass.m_handle = VK_NULL_HANDLE;

	pipeline pipeline = {};
	pipeline.add_shader(vertex_shader_module);
	pipeline.add_shader(fragment_shader_module);
	pipeline.build(m_device, pipeline_layout, render_pass, m_wsi.m_swapchain.m_extent);

	command_pool command_pool = {};
	command_pool.build(m_device);
	std::vector<command_buffer> command_buffers(m_wsi.m_swapchain.m_images.size());
	for (auto &command_buffer : command_buffers)
	{
		command_buffer.build(m_device, command_pool);
	}

	for (size_t i = 0; i < command_buffers.size(); i++)
	{
		command_buffers[i].begin();
		{
			VkClearValue clear_color = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
			VkRenderingAttachmentInfo color_attachment = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,      //
				.pNext = nullptr,                                          //
				.imageView = m_wsi.m_swapchain.m_image_views[i]->m_handle, //
				.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,   //
				.resolveMode = VK_RESOLVE_MODE_NONE,                       //
				.resolveImageView = VK_NULL_HANDLE,                        //
				.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,           //
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,                     //
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,                   //
				.clearValue = clear_color,                                 //
			};
			VkRenderingInfo rendering_info = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,              //
				.pNext = nullptr,                                       //
				.flags = 0,                                             //
				.renderArea = { { 0, 0 }, m_wsi.m_swapchain.m_extent }, //
				.layerCount = 1,                                        //
				.viewMask = 0,                                          //
				.colorAttachmentCount = 1,                              //
				.pColorAttachments = &color_attachment,                 //
				.pDepthAttachment = nullptr,                            //
				.pStencilAttachment = nullptr,                          //
			};

			vkCmdBeginRendering(command_buffers[i].m_handle, &rendering_info);
			{
				vkCmdBindPipeline(command_buffers[i].m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.m_handle);
				vkCmdDraw(command_buffers[i].m_handle, 3, 1, 0, 0);
			}
			vkCmdEndRendering(command_buffers[i].m_handle);
		}
		command_buffers[i].end();
	}

	constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;

	std::vector<semaphore> imageAvailableSemaphores(MAX_FRAMES_IN_FLIGHT);
	std::vector<semaphore> renderFinishedSemaphores(MAX_FRAMES_IN_FLIGHT);
	std::vector<fence> frameFences(MAX_FRAMES_IN_FLIGHT);
	std::vector<fence *> imageFences(m_wsi.m_swapchain.m_images.size(), nullptr);
	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		imageAvailableSemaphores[i].build(m_device);
		renderFinishedSemaphores[i].build(m_device);
		frameFences[i].build(m_device);
	}

	u32 frame = 0u;
	while (m_window.step())
	{
		frameFences[frame].wait();

		uint32_t imageIndex{};
		m_wsi.acquire_image(imageAvailableSemaphores[frame], &imageIndex);

		if (imageFences[imageIndex] != nullptr)
		{
			imageFences[imageIndex]->wait();
		}
		imageFences[imageIndex] = &frameFences[frame];

		frameFences[frame].reset();
		m_queue.submit(command_buffers[imageIndex], imageAvailableSemaphores[frame], renderFinishedSemaphores[frame],
		               frameFences[frame]);
		m_queue.present(renderFinishedSemaphores[frame], m_wsi, imageIndex);

		frame = (frame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	m_device.wait();
}

} /* namespace vulkan */
