#include <algorithm>
#include <iostream>
#include <optional>
#include <string_view>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <model/model.h>
#include <platform/window.h>
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
};
static_assert(sizeof(uniforms) == 4 * 4 * 4 * 3, "Unexpected struct uniform size");

namespace vulkan
{

context::~context()
{
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
	m_window.init(800, 600);
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
	model model = {};
	model.load("bin/assets/models/DamagedHelmet.glb");

	/* Vertex input. */
	buffer vertex_buffer = {};
	m_resource_allocator.allocate_buffer(vertex_buffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                                     sizeof(model.m_meshes[0].m_vertices[0]) * model.m_meshes[0].m_vertices.size());
	vertex_buffer.fill(model.m_meshes[0].m_vertices.data(),
	                   sizeof(model.m_meshes[0].m_vertices[0]) * model.m_meshes[0].m_vertices.size());
	buffer index_buffer = {};
	m_resource_allocator.allocate_buffer(index_buffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	                                     sizeof(model.m_meshes[0].m_indices[0]) * model.m_meshes[0].m_indices.size());
	index_buffer.fill(model.m_meshes[0].m_indices.data(),
	                  sizeof(model.m_meshes[0].m_indices[0]) * model.m_meshes[0].m_indices.size());

	/* Uniform buffer. */
	uniforms uniforms = {};
	uniforms.model = glm::mat4(1.0f);
	uniforms.view = glm::lookAt(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	uniforms.projection = glm::perspective(glm::radians(45.0f), 800 / (float)600, 0.1f, 10.0f);
	uniforms.projection[1][1] *= -1.0f;
	buffer uniform_buffer = {};
	m_resource_allocator.allocate_buffer(uniform_buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(uniforms));
	uniform_buffer.fill(&uniforms, sizeof(uniforms));

	/* Texture. */
	image texture = {};
	m_resource_allocator.allocate_image(texture, VK_FORMAT_R8G8B8A8_SRGB,
	                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 32, 32);
	texture.fill(*this, model.m_meshes[0].m_texture.data(), model.m_meshes[0].m_width * 4);

	shader_module vertex_shader_module = {};
	vertex_shader_module.build(m_device, VK_SHADER_STAGE_VERTEX_BIT, "bin/assets/shaders/basic.vert.spv");

	shader_module fragment_shader_module = {};
	fragment_shader_module.build(m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "bin/assets/shaders/basic.frag.spv");

	pipeline_layout pipeline_layout = {};
	pipeline_layout.build(m_device);

	render_pass render_pass = {};
	render_pass.use_dynamic_rendering();
	render_pass.build(m_device, VK_FORMAT_UNDEFINED);

	pipeline pipeline = {};
	pipeline.add_shader(vertex_shader_module);
	pipeline.add_shader(fragment_shader_module);
	pipeline.build(m_device, render_pass, m_wsi.m_swapchain.m_extent);

	for (std::unique_ptr<image> &image : m_wsi.m_swapchain.m_images)
	{
		image->transition_layout(*this, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	std::vector<command_buffer> command_buffers(m_wsi.m_swapchain.m_images.size());
	for (auto &command_buffer : command_buffers)
	{
		command_buffer.build(m_device, m_command_pool);
	}

	for (size_t i = 0; i < command_buffers.size(); i++)
	{
		command_buffers[i].begin();
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
				.image = m_wsi.m_swapchain.m_vulkan_images[i],                   //
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
			vkCmdPipelineBarrier2(command_buffers[i].m_handle, &begin_rendering_dependency_info);

			/* Create render pass. */
			const VkClearValue clear_color = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
			const VkRenderingAttachmentInfo color_attachment = {
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
			const VkRenderingInfo rendering_info = {
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

			/* Render. */
			vkCmdBeginRendering(command_buffers[i].m_handle, &rendering_info);
			{
				vkCmdBindPipeline(command_buffers[i].m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.m_handle);
				constexpr VkDeviceSize offset = 0;
				vkCmdBindVertexBuffers(command_buffers[i].m_handle, 0, 1, &vertex_buffer.m_handle, &offset);
				vkCmdBindIndexBuffer(command_buffers[i].m_handle, index_buffer.m_handle, 0, VK_INDEX_TYPE_UINT32);

				/* (TODO, thoave01): Add to command buffer as methods. E.g. set_buffer(). */
				VkDescriptorBufferInfo buffer_info = {};
				buffer_info.buffer = uniform_buffer.m_handle;
				buffer_info.offset = 0;
				buffer_info.range = VK_WHOLE_SIZE;
				VkWriteDescriptorSet write = {};
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.pNext = nullptr;
				write.dstSet = 0;
				write.dstBinding = 0;
				write.dstArrayElement = 0;
				write.descriptorCount = 1;
				write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				write.pBufferInfo = &buffer_info;
				/* (TODO, thoave01): We have to actually make a pipeline layout at some point. */
				vkCmdPushDescriptorSetKHR(command_buffers[i].m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS,
				                          pipeline.m_pipeline_layout.m_handle, 0, 1, &write);

				vkCmdDraw(command_buffers[i].m_handle, 3, 1, 0, 0);
			}
			vkCmdEndRendering(command_buffers[i].m_handle);

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
				.image = m_wsi.m_swapchain.m_vulkan_images[i],                   //
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
			vkCmdPipelineBarrier2(command_buffers[i].m_handle, &end_rendering_dependency_info);
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
