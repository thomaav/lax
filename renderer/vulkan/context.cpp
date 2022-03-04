#include <algorithm>
#include <iostream>
#include <optional>
#include <string_view>

#include <platform/window.h>
#include <renderer/vulkan/command_buffer.h>
#include <renderer/vulkan/command_pool.h>
#include <renderer/vulkan/context.h>
#include <renderer/vulkan/pipeline.h>
#include <renderer/vulkan/render_pass.h>
#include <renderer/vulkan/semaphore.h>
#include <renderer/vulkan/shader.h>
#include <renderer/vulkan/util.h>
#include <utils/type.h>
#include <utils/util.h>

namespace Vulkan
{

Context::~Context()
{
	/* Destroy device. */
	wsi.destroySwapchain();
	device.destroy();

	/* Destroy instance. */
	wsi.destroySurface();
	instance.destroy();
}

void Context::addInstanceExtension(const char *extension)
{
	instance.addExtension(extension);
}

void Context::addInstanceLayer(const char *layer)
{
	instance.addLayer(layer);
}

void Context::addDeviceExtension(const char *extension)
{
	/* (TODO, thoave01): Assert if we already built the context. */
	device.addExtension(extension);
}

/* (TODO, thoave01): Blabla platform window can be templated. Fuck it use glfwWindow. */
void Context::build()
{
	/* Instance initialization. */
	{
		/* (TODO, thoave01): Some error handling? */
		instance.build();

		/* (TODO, thoave01): WSI should be optional. */
		/* (TODO, thoave01): WSI width/height or something. */
		wsi.buildSurface(instance);
	}

	/* Device initialization. */
	{
		device.build(instance, wsi.surface.handle);
		wsi.buildSwapchain(device);
		queue.build(device);
	}

	Shader vertexShader{};
	vertexShader.build(device, VK_SHADER_STAGE_VERTEX_BIT, "bin/shaders/basic.vert.spv");

	Shader fragmentShader{};
	fragmentShader.build(device, VK_SHADER_STAGE_FRAGMENT_BIT, "bin/shaders/basic.frag.spv");

	/* Create a pipeline. */
	{
		PipelineLayout pipelineLayout{};
		pipelineLayout.build(device);

		RenderPass renderPass{};
		renderPass.build(device, wsi.swapchain.format);

		Pipeline pipeline{};
		pipeline.addShader(vertexShader);
		pipeline.addShader(fragmentShader);
		pipeline.build(device, pipelineLayout, renderPass, wsi.swapchain.extent);

		std::vector<Framebuffer> swapchainFramebuffers(wsi.swapchain.images.size());
		for (u32 i = 0; i < wsi.swapchain.images.size(); i++)
		{
			swapchainFramebuffers[i].addColorAttachment(*wsi.swapchain.imageViews[i]);
			swapchainFramebuffers[i].build(device, renderPass);
		}

		CommandPool commandPool{};
		commandPool.build(device);

		std::vector<CommandBuffer> commandBuffers(wsi.swapchain.images.size());
		for (auto &commandBuffer : commandBuffers)
		{
			commandBuffer.build(device, commandPool);
		}

		/* Record command buffer(s). */
		for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			commandBuffers[i].begin();
			{
				VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = renderPass.handle;
				renderPassInfo.framebuffer = swapchainFramebuffers[i].handle;

				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = wsi.swapchain.extent;

				VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
				renderPassInfo.clearValueCount = 1;
				renderPassInfo.pClearValues = &clearColor;

				vkCmdBeginRenderPass(commandBuffers[i].handle, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
				{
					vkCmdBindPipeline(commandBuffers[i].handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
					vkCmdDraw(commandBuffers[i].handle, 3, 1, 0, 0);
				}
				vkCmdEndRenderPass(commandBuffers[i].handle);
			}
			commandBuffers[i].end();
		}

		/* Draw some stuff. */
		Semaphore imageAvailableSemaphore{};
		Semaphore renderFinishedSemaphore{};
		{
			imageAvailableSemaphore.build(device);
			renderFinishedSemaphore.build(device);

			while (wsi.window.handle.step())
			{
				uint32_t imageIndex{};
				vkAcquireNextImageKHR(device.logical.handle, wsi.swapchain.handle, UINT64_MAX,
				                      imageAvailableSemaphore.handle, VK_NULL_HANDLE, &imageIndex);

				queue.submit(commandBuffers[imageIndex], imageAvailableSemaphore, renderFinishedSemaphore);

				VkPresentInfoKHR presentInfo{};
				presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

				VkSemaphore signalSemaphores[] = { renderFinishedSemaphore.handle };
				presentInfo.waitSemaphoreCount = 1;
				presentInfo.pWaitSemaphores = signalSemaphores;

				VkSwapchainKHR swapchains[] = { wsi.swapchain.handle };
				presentInfo.swapchainCount = 1;
				presentInfo.pSwapchains = swapchains;
				presentInfo.pImageIndices = &imageIndex;
				presentInfo.pResults = nullptr;
				vkQueuePresentKHR(queue.handle, &presentInfo);

				/* (TODO, thoave01): Fix this. */
				queue.wait();
			}
		}

		device.wait();
	}

	fragmentShader.destroy();
	vertexShader.destroy();
}

} /* namespace Vulkan */
