#include <algorithm>
#include <iostream>
#include <optional>
#include <string_view>

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

/* (TODO, thoave01): Remove later. */
#include <glm/glm.hpp>

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

struct vertex
{
	glm::vec2 pos;
	glm::vec3 color;
};

void context::build()
{
	/* Instance initialization. */
	m_instance.build();
	m_wsi.build_surface(m_instance);

	/* Device initialization. */
	m_device.build(m_instance, m_wsi.m_surface.handle);
	m_wsi.build_swapchain(m_device);
	m_queue.build(m_device);
}

void context::backend_test()
{
	shader_module vertex_shader_module = {};
	vertex_shader_module.build(m_device, VK_SHADER_STAGE_VERTEX_BIT, "bin/shaders/basic.vert.spv");

	shader_module fragment_shader_module = {};
	fragment_shader_module.build(m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "bin/shaders/basic.frag.spv");

	/* Create a pipeline. */
	{
		pipeline_layout pipelineLayout{};
		pipelineLayout.build(m_device);

		render_pass renderPass{};
		renderPass.build(m_device, m_wsi.m_swapchain.m_format);

		pipeline pipeline{};
		{
			pipeline.add_shader(vertex_shader_module);
			pipeline.add_shader(fragment_shader_module);

			pipeline.add_vertex_binding(0, sizeof(vertex));
			pipeline.add_vertex_attribute(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, pos));
			pipeline.add_vertex_attribute(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, color));
		}
		pipeline.build(m_device, pipelineLayout, renderPass, m_wsi.m_swapchain.m_extent);

		std::vector<framebuffer> swapchainFramebuffers(m_wsi.m_swapchain.m_images.size());
		for (u32 i = 0; i < m_wsi.m_swapchain.m_images.size(); i++)
		{
			swapchainFramebuffers[i].add_color_attachment(*m_wsi.m_swapchain.m_image_views[i]);
			swapchainFramebuffers[i].build(m_device, renderPass);
		}

		command_pool commandPool{};
		commandPool.build(m_device);

		std::vector<command_buffer> commandBuffers(m_wsi.m_swapchain.m_images.size());
		for (auto &commandBuffer : commandBuffers)
		{
			commandBuffer.build(m_device, commandPool);
		}

		for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			commandBuffers[i].begin();
			{
				VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = renderPass.m_handle;
				renderPassInfo.framebuffer = swapchainFramebuffers[i].m_handle;

				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = m_wsi.m_swapchain.m_extent;

				VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
				renderPassInfo.clearValueCount = 1;
				renderPassInfo.pClearValues = &clearColor;

				vkCmdBeginRenderPass(commandBuffers[i].m_handle, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
				{
					vkCmdBindPipeline(commandBuffers[i].m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.m_handle);
					vkCmdDraw(commandBuffers[i].m_handle, 3, 1, 0, 0);
				}
				vkCmdEndRenderPass(commandBuffers[i].m_handle);
			}
			commandBuffers[i].end();
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
		while (m_wsi.window.m_handle.step())
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
			m_queue.submit(commandBuffers[imageIndex], imageAvailableSemaphores[frame], renderFinishedSemaphores[frame],
			               frameFences[frame]);
			m_queue.present(renderFinishedSemaphores[frame], m_wsi, imageIndex);

			frame = (frame + 1) % MAX_FRAMES_IN_FLIGHT;
		}

		m_device.wait();
	}
}

} /* namespace vulkan */
