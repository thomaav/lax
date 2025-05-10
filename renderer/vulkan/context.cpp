#include <algorithm>
#include <chrono>
#include <iostream>
#include <optional>
#include <string_view>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assets/image.h>
#include <assets/model.h>
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
	assets::model model = {};
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
	uniforms.model = model.m_meshes[0].m_transform;
	uniforms.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	uniforms.projection = glm::perspectiveRH_ZO(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 256.0f);
	uniforms.projection[1][1] *= -1.0f; /* (TODO, thoave01): Should flip viewport instead. KHR_maintenance1. */
	buffer uniform_buffer = {};
	m_resource_allocator.allocate_buffer(uniform_buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(uniforms));
	uniform_buffer.fill(&uniforms, sizeof(uniforms));

	/* (TODO, thoave01): Move image inside texture and allocator. */
	/* Texture. */
	image texture_image = {};
	m_resource_allocator.allocate_image_2d(texture_image, VK_FORMAT_R8G8B8A8_SRGB,
	                                       VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	                                       model.m_meshes[0].m_width, model.m_meshes[0].m_height);
	texture_image.fill(*this, model.m_meshes[0].m_texture.data(), model.m_meshes[0].m_texture.size());
	texture_image.transition_layout(*this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	texture color_texture = {};
	color_texture.build(m_device, texture_image);

	/* Depth texture. */
	image depth_texture_image = {};
	m_resource_allocator.allocate_image_2d(depth_texture_image, VK_FORMAT_D32_SFLOAT,
	                                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	                                       m_wsi.m_swapchain.m_extent.width, m_wsi.m_swapchain.m_extent.height);
	depth_texture_image.transition_layout(*this, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	texture depth_texture = {};
	depth_texture.build(m_device, depth_texture_image);

	/* Skybox. */
	assets::image skybox_asset_image = {};
	skybox_asset_image.load("bin/assets/images/skybox/right.jpg");
	image skybox_image = {};
	m_resource_allocator.allocate_image_layered(skybox_image, VK_FORMAT_R8G8B8A8_SRGB,
	                                            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	                                            skybox_asset_image.m_width, skybox_asset_image.m_height, 6);
	skybox_asset_image.load("bin/assets/images/skybox/right.jpg");
	skybox_image.fill_layer(*this, skybox_asset_image.m_data.data(), skybox_asset_image.m_data.size(), 0);
	skybox_asset_image.load("bin/assets/images/skybox/left.jpg");
	skybox_image.fill_layer(*this, skybox_asset_image.m_data.data(), skybox_asset_image.m_data.size(), 1);
	skybox_asset_image.load("bin/assets/images/skybox/top.jpg");
	skybox_image.fill_layer(*this, skybox_asset_image.m_data.data(), skybox_asset_image.m_data.size(), 2);
	skybox_asset_image.load("bin/assets/images/skybox/bottom.jpg");
	skybox_image.fill_layer(*this, skybox_asset_image.m_data.data(), skybox_asset_image.m_data.size(), 3);
	skybox_asset_image.load("bin/assets/images/skybox/front.jpg");
	skybox_image.fill_layer(*this, skybox_asset_image.m_data.data(), skybox_asset_image.m_data.size(), 4);
	skybox_asset_image.load("bin/assets/images/skybox/back.jpg");
	skybox_image.fill_layer(*this, skybox_asset_image.m_data.data(), skybox_asset_image.m_data.size(), 5);
	skybox_image.transition_layout(*this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	texture skybox_texture = {};
	skybox_texture.build(m_device, skybox_image);

	render_pass render_pass = {};
	render_pass.use_dynamic_rendering();
	render_pass.build(m_device, m_wsi.m_swapchain.m_images[0]->m_format, VK_FORMAT_D32_SFLOAT);

	shader_module vertex_shader_module = {};
	vertex_shader_module.build(m_device, VK_SHADER_STAGE_VERTEX_BIT, "bin/assets/shaders/basic.vert.spv");
	shader_module fragment_shader_module = {};
	fragment_shader_module.build(m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "bin/assets/shaders/basic.frag.spv");
	pipeline basic_pipeline = {};
	basic_pipeline.add_shader(vertex_shader_module);
	basic_pipeline.add_shader(fragment_shader_module);
	basic_pipeline.build(m_device, render_pass, m_wsi.m_swapchain.m_extent);

	shader_module vertex_skybox_shader_module = {};
	vertex_skybox_shader_module.build(m_device, VK_SHADER_STAGE_VERTEX_BIT, "bin/assets/shaders/skybox.vert.spv");
	shader_module fragment_skybox_shader_module = {};
	fragment_skybox_shader_module.build(m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "bin/assets/shaders/skybox.frag.spv");
	pipeline skybox_pipeline = {};
	skybox_pipeline.add_shader(vertex_skybox_shader_module);
	skybox_pipeline.add_shader(fragment_skybox_shader_module);
	skybox_pipeline.build(m_device, render_pass, m_wsi.m_swapchain.m_extent);

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

	while (m_window.step())
	{
		static auto start_time = std::chrono::high_resolution_clock::now();
		auto current_time = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
		glm::mat4 time_rotation = glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		uniforms.model = time_rotation * model.m_meshes[0].m_transform;
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
				command_buffer.bind_pipeline(basic_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
				constexpr VkDeviceSize offset = 0;
				vkCmdBindVertexBuffers(command_buffer.m_handle, 0, 1, &vertex_buffer.m_handle, &offset);
				vkCmdBindIndexBuffer(command_buffer.m_handle, index_buffer.m_handle, 0, VK_INDEX_TYPE_UINT32);
				command_buffer.set_uniform_buffer(0, uniform_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
				command_buffer.set_texture(1, color_texture, VK_PIPELINE_BIND_POINT_GRAPHICS);
				vkCmdDrawIndexed(command_buffer.m_handle, model.m_meshes[0].m_indices.size(),
				                 /* instanceCount = */ 1, /* firstIndex = */ 0, /* vertexOffset = */ 0,
				                 /* firstInstance = */ 0);

				command_buffer.bind_pipeline(skybox_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
				command_buffer.set_uniform_buffer(0, uniform_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
				command_buffer.set_texture(1, skybox_texture, VK_PIPELINE_BIND_POINT_GRAPHICS);
				vkCmdDraw(command_buffer.m_handle, 36, 1, /* firstVertex = */ 0, /* firstInstance = */ 0);
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
