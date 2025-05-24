#include <algorithm>

#include <platform/input.h>
#include <renderer/vulkan/context.h>

#include "log.h"
#include "object.h"

void skybox::build(vulkan::context &context)
{
	/* Load image to get dimensions. */
	m_asset_image.load("bin/assets/images/skybox/right.jpg");
	m_texture.build(context, {
	                             .m_format = VK_FORMAT_R8G8B8A8_SRGB,
	                             .m_width = (u32)m_asset_image.m_width,
	                             .m_height = (u32)m_asset_image.m_height,
	                             .m_usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	                             .m_layers = 6,
	                         });

	/* (TODO, thoave01): Add `fill` etc. to texture as well. */
	m_asset_image.load("bin/assets/images/skybox/right.jpg");
	m_texture.m_image.fill_layer(context, m_asset_image.m_data.data(), m_asset_image.m_data.size(), 0);
	m_asset_image.load("bin/assets/images/skybox/left.jpg");
	m_texture.m_image.fill_layer(context, m_asset_image.m_data.data(), m_asset_image.m_data.size(), 1);
	m_asset_image.load("bin/assets/images/skybox/top.jpg");
	m_texture.m_image.fill_layer(context, m_asset_image.m_data.data(), m_asset_image.m_data.size(), 2);
	m_asset_image.load("bin/assets/images/skybox/bottom.jpg");
	m_texture.m_image.fill_layer(context, m_asset_image.m_data.data(), m_asset_image.m_data.size(), 3);
	m_asset_image.load("bin/assets/images/skybox/front.jpg");
	m_texture.m_image.fill_layer(context, m_asset_image.m_data.data(), m_asset_image.m_data.size(), 4);
	m_asset_image.load("bin/assets/images/skybox/back.jpg");
	m_texture.m_image.fill_layer(context, m_asset_image.m_data.data(), m_asset_image.m_data.size(), 5);
	m_texture.m_image.transition_layout(context, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	/* Pipeline. */
	m_pipeline.add_shader(context.m_device, VK_SHADER_STAGE_VERTEX_BIT, "bin/assets/shaders/skybox.vert.spv");
	m_pipeline.add_shader(context.m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "bin/assets/shaders/skybox.frag.spv");
	m_pipeline.build(context.m_device);

	/* Uniforms. */
	m_uniforms.model = glm::mat4(1.0f);
}

void skybox::draw(vulkan::command_buffer &command_buffer)
{
	/* (TODO, thoave01): Fix stage flags. */
	vkCmdPushConstants(command_buffer.m_handle, m_pipeline.m_pipeline_layout.m_handle,
	                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, /* offset = */ 0,
	                   m_pipeline.m_pipeline_layout.m_push_constants_size, &m_uniforms);

	command_buffer.bind_pipeline(m_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
	command_buffer.set_texture(1, m_texture, VK_PIPELINE_BIND_POINT_GRAPHICS);
	vkCmdDraw(command_buffer.m_handle, 36, 1, /* firstVertex = */ 0, /* firstInstance = */ 0);
}

void skybox::update_material(VkSampleCountFlagBits sample_count)
{
	m_pipeline.set_sample_count(sample_count);
	m_pipeline.update();
}

void static_mesh::build(vulkan::context &context, ref<assets::model> model)
{
	m_model = model;

	/* Vertex buffer. */
	m_vertex_buffer = context.m_resource_allocator.allocate_buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                                                               sizeof(m_model->m_meshes[0].m_vertices[0]) *
	                                                                   m_model->m_meshes[0].m_vertices.size());
	m_vertex_buffer.fill(m_model->m_meshes[0].m_vertices.data(),
	                     sizeof(m_model->m_meshes[0].m_vertices[0]) * m_model->m_meshes[0].m_vertices.size());

	/* Index buffer. */
	m_index_buffer = context.m_resource_allocator.allocate_buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	                                                              sizeof(m_model->m_meshes[0].m_indices[0]) *
	                                                                  m_model->m_meshes[0].m_indices.size());
	m_index_buffer.fill(m_model->m_meshes[0].m_indices.data(),
	                    sizeof(m_model->m_meshes[0].m_indices[0]) * m_model->m_meshes[0].m_indices.size());
	m_index_count = m_model->m_meshes[0].m_indices.size();

	/* Diffuse texture. */
	m_diffuse_texture.build(context, { .m_format = VK_FORMAT_R8G8B8A8_SRGB,
	                                   .m_width = (u32)m_model->m_meshes[0].m_width,
	                                   .m_height = (u32)m_model->m_meshes[0].m_height,
	                                   .m_usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	                                              VK_IMAGE_USAGE_SAMPLED_BIT,
	                                   .m_mipmapped = true });
	m_diffuse_texture.m_image.fill(context, m_model->m_meshes[0].m_texture.data(),
	                               m_model->m_meshes[0].m_texture.size());
	m_diffuse_texture.m_image.generate_mipmaps(context);
	m_diffuse_texture.m_image.transition_layout(context, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	/* Pipeline. */
	m_pipeline.add_shader(context.m_device, VK_SHADER_STAGE_VERTEX_BIT, "bin/assets/shaders/basic.vert.spv");
	m_pipeline.add_shader(context.m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "bin/assets/shaders/basic.frag.spv");
	m_pipeline.build(context.m_device);

	/* Uniforms. */
	m_uniforms.model = m_model->m_meshes[0].m_transform;
}

void static_mesh::draw(vulkan::command_buffer &command_buffer)
{
	/* (TODO, thoave01): Fix stage flags. */
	vkCmdPushConstants(command_buffer.m_handle, m_pipeline.m_pipeline_layout.m_handle,
	                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, /* offset = */ 0,
	                   m_pipeline.m_pipeline_layout.m_push_constants_size, &m_uniforms);

	command_buffer.bind_pipeline(m_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
	constexpr VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(command_buffer.m_handle, 0, 1, &m_vertex_buffer.m_handle, &offset);
	vkCmdBindIndexBuffer(command_buffer.m_handle, m_index_buffer.m_handle, 0, VK_INDEX_TYPE_UINT32);
	command_buffer.set_texture(1, m_diffuse_texture, VK_PIPELINE_BIND_POINT_GRAPHICS);
	vkCmdDrawIndexed(command_buffer.m_handle, m_index_count,
	                 /* instanceCount = */ 1, /* firstIndex = */ 0, /* vertexOffset = */ 0,
	                 /* firstInstance = */ 0);
}

void static_mesh::update_material(VkSampleCountFlagBits sample_count)
{
	m_pipeline.set_sample_count(sample_count);
	m_pipeline.update();
}

void camera::build(glm::vec3 position, glm::vec3 target)
{
	m_position = position;
	m_forward = glm::normalize(target - position);

	/* (TODO, thoave01): Pitch up/down glitches camera over 90 degrees. */
	const glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);
	const glm::vec3 right = glm::normalize(glm::cross(m_forward, world_up));
	m_up = glm::vec3(0.0f, 1.0f, 0.0f);
	m_yaw = glm::degrees(atan2(m_forward.z, m_forward.x));
	m_pitch = glm::degrees(asin(m_forward.y));
	m_view = glm::lookAt(m_position, m_position + m_forward, m_up);

	m_fov = glm::radians(75.0f);
	m_aspect_ratio = 16.0f / 9.0f;
	m_near = 0.1f;
	m_far = 256.0f;
	m_projection = glm::perspectiveRH_ZO(m_fov, m_aspect_ratio, m_near, m_far);
}

void camera::update(float aspect_ratio)
{
	m_aspect_ratio = aspect_ratio;

	bool w_pressed = input::is_key_pressed(GLFW_KEY_W);
	bool a_pressed = input::is_key_pressed(GLFW_KEY_A);
	bool s_pressed = input::is_key_pressed(GLFW_KEY_S);
	bool d_pressed = input::is_key_pressed(GLFW_KEY_D);
	double mouse_x, mouse_y;
	input::get_mouse_position(mouse_x, mouse_y);
	bool right_mouse_pressed = input::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);
	bool middle_mouse_pressed = input::is_mouse_button_pressed(GLFW_MOUSE_BUTTON_MIDDLE);

	static float last_frame = glfwGetTime();
	float current_frame = glfwGetTime();
	float delta_frame = current_frame - last_frame;
	float camera_speed = m_speed * delta_frame;
	last_frame = current_frame;

	static double last_mouse_x = mouse_x;
	static double last_mouse_y = mouse_y;
	if (right_mouse_pressed)
	{
		m_yaw += (mouse_x - last_mouse_x) * m_sensitivity * delta_frame;
		m_pitch += std::clamp((last_mouse_y - mouse_y) * m_sensitivity * delta_frame, -89.0, 89.0);
		m_forward.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		m_forward.y = sin(glm::radians(m_pitch));
		m_forward.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	}
	if (middle_mouse_pressed)
	{
		m_position += m_up * (float)((last_mouse_y - mouse_y) * m_speed / 5.0f * delta_frame);
		m_position += glm::normalize(glm::cross(m_forward, m_up)) *
		              (float)((mouse_x - last_mouse_x) * m_speed / 5.0f * delta_frame);
	}
	last_mouse_x = mouse_x;
	last_mouse_y = mouse_y;

	if (w_pressed)
	{
		m_position += m_forward * camera_speed;
	}
	if (a_pressed)
	{
		m_position -= glm::normalize(glm::cross(m_forward, m_up)) * camera_speed;
	}
	if (s_pressed)
	{
		m_position -= m_forward * camera_speed;
	}
	if (d_pressed)
	{
		m_position += glm::normalize(glm::cross(m_forward, m_up)) * camera_speed;
	}

	m_view = glm::lookAt(m_position, m_position + m_forward, m_up);
	m_projection = glm::perspectiveRH_ZO(m_fov, m_aspect_ratio, m_near, m_far);
}

void camera::draw(vulkan::command_buffer &command_buffer)
{
}
