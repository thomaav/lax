#include <chrono>

#include <renderer/scene.h>

void skybox::build(vulkan::context &context, const vulkan::render_pass &render_pass)
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
	m_pipeline.build(context.m_device, render_pass);

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

void skybox::update_material(const vulkan::render_pass &render_pass, VkSampleCountFlagBits sample_count)
{
	m_pipeline.set_sample_count(sample_count);
	m_pipeline.update(render_pass);
}

void static_mesh::build(vulkan::context &context, const vulkan::render_pass &render_pass, ref<assets::model> model)
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
	m_pipeline.build(context.m_device, render_pass);
}

void static_mesh::draw(vulkan::command_buffer &command_buffer)
{
	/* (TODO, thoave01): Fix stage flags. */
	static auto start_time = std::chrono::high_resolution_clock::now();
	auto current_time = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
	glm::mat4 time_rotation = glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_uniforms.model = time_rotation * m_model->m_meshes[0].m_transform;
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

void static_mesh::update_material(const vulkan::render_pass &render_pass, VkSampleCountFlagBits sample_count)
{
	m_pipeline.set_sample_count(sample_count);
	m_pipeline.update(render_pass);
}

void node::add_child(const ref<object> &child)
{
	node child_node = {};
	child_node.m_object = child;
	m_children.push_back(child_node);
}

void scene::build_default_scene(vulkan::context &context, const vulkan::render_pass &render_pass)
{
	ref<assets::model> model = make_ref<assets::model>();
	model->load("bin/assets/models/DamagedHelmet.glb");
	ref<static_mesh> static_mesh_ = make_ref<static_mesh>();
	static_mesh_->build(context, render_pass, model);
	m_root.add_child(std::static_pointer_cast<object>(static_mesh_));

	ref<skybox> skybox_ = make_ref<skybox>();
	skybox_->build(context, render_pass);
	m_root.add_child(std::static_pointer_cast<object>(skybox_));

	m_skybox = skybox_;
	m_static_mesh = static_mesh_;

	m_uniform_buffer =
	    context.m_resource_allocator.allocate_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(m_uniforms));
}
