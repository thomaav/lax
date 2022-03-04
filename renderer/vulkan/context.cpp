#include <algorithm>
#include <iostream>
#include <optional>
#include <string_view>

#include <platform/window.h>
#include <renderer/vulkan/command_buffer.h>
#include <renderer/vulkan/command_pool.h>
#include <renderer/vulkan/context.h>
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

	/* (TODO, thoave01): Figure out how to represent this thing. */
	/* Create a pipeline. */
	{
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)wsi.swapchain.extent.width;
		viewport.height = (float)wsi.swapchain.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = wsi.swapchain.extent;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask =
		    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		/* Create pipeline layout. */
		VkPipelineLayout pipelineLayout{};
		{
			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 0;
			pipelineLayoutInfo.pSetLayouts = nullptr;
			pipelineLayoutInfo.pushConstantRangeCount = 0;
			pipelineLayoutInfo.pPushConstantRanges = nullptr;

			VULKAN_ASSERT_SUCCESS(
			    vkCreatePipelineLayout(device.logical.handle, &pipelineLayoutInfo, nullptr, &pipelineLayout));
		}

		/* Create renderpass. */
		RenderPass renderPass{};
		renderPass.build(device, wsi.swapchain.format);

		VkPipelineShaderStageCreateInfo stages[2] = { fragmentShader.getPipelineShaderStageCreateInfo(),
			                                          vertexShader.getPipelineShaderStageCreateInfo() };

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = stages;

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;

		pipelineInfo.layout = pipelineLayout;

		pipelineInfo.renderPass = renderPass.handle;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VkPipeline graphicsPipeline{};
		vkCreateGraphicsPipelines(device.logical.handle, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);

		/* Create framebuffer(s). */
		std::vector<Framebuffer> swapchainFramebuffers(wsi.swapchain.images.size());
		for (u32 i = 0; i < wsi.swapchain.images.size(); i++)
		{
			swapchainFramebuffers[i].addColorAttachment(*wsi.swapchain.imageViews[i]);
			swapchainFramebuffers[i].build(device, renderPass);
		}

		CommandPool commandPool{};
		commandPool.build(device);

		/* Create command buffer(s). */
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
					vkCmdBindPipeline(commandBuffers[i].handle, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
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

				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

				VkSemaphore waitSemaphores[] = { imageAvailableSemaphore.handle };
				VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
				submitInfo.waitSemaphoreCount = 1;
				submitInfo.pWaitSemaphores = waitSemaphores;
				submitInfo.pWaitDstStageMask = waitStages;

				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &commandBuffers[imageIndex].handle;

				VkSemaphore signalSemaphores[] = { renderFinishedSemaphore.handle };
				submitInfo.signalSemaphoreCount = 1;
				submitInfo.pSignalSemaphores = signalSemaphores;

				VULKAN_ASSERT_SUCCESS(vkQueueSubmit(queue.handle, 1, &submitInfo, VK_NULL_HANDLE));

				VkPresentInfoKHR presentInfo{};
				presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

				presentInfo.waitSemaphoreCount = 1;
				presentInfo.pWaitSemaphores = signalSemaphores;

				VkSwapchainKHR swapchains[] = { wsi.swapchain.handle };
				presentInfo.swapchainCount = 1;
				presentInfo.pSwapchains = swapchains;
				presentInfo.pImageIndices = &imageIndex;
				presentInfo.pResults = nullptr;
				vkQueuePresentKHR(queue.handle, &presentInfo);

				queue.wait();
			}
		}

		device.wait();

		vkDestroyPipeline(device.logical.handle, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device.logical.handle, pipelineLayout, nullptr);
	}

	fragmentShader.destroy();
	vertexShader.destroy();
}

} /* namespace Vulkan */
