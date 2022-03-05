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

		constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;

		std::vector<Semaphore> imageAvailableSemaphores(MAX_FRAMES_IN_FLIGHT);
		std::vector<Semaphore> renderFinishedSemaphores(MAX_FRAMES_IN_FLIGHT);
		std::vector<Fence> frameFences(MAX_FRAMES_IN_FLIGHT);
		std::vector<Fence *> imageFences(wsi.swapchain.images.size(), nullptr);
		for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			imageAvailableSemaphores[i].build(device);
			renderFinishedSemaphores[i].build(device);
			frameFences[i].build(device);
		}

		u32 frame = 0u;
		while (wsi.window.handle.step())
		{
			frameFences[frame].wait();

			uint32_t imageIndex{};
			wsi.acquireImage(imageAvailableSemaphores[frame], &imageIndex);

			if (imageFences[imageIndex] != nullptr)
			{
				imageFences[imageIndex]->wait();
			}
			imageFences[imageIndex] = &frameFences[frame];

			frameFences[frame].reset();
			queue.submit(commandBuffers[imageIndex], imageAvailableSemaphores[frame], renderFinishedSemaphores[frame],
			             frameFences[frame]);
			queue.present(renderFinishedSemaphores[frame], wsi, imageIndex);

			frame = (frame + 1) % MAX_FRAMES_IN_FLIGHT;
		}

		device.wait();
	}

	fragmentShader.destroy();
	vertexShader.destroy();
}

} /* namespace Vulkan */
