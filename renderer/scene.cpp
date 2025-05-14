#include <renderer/scene.h>

void skybox::build(vulkan::context &context, const vulkan::render_pass &render_pass)
{
	/* Vulkan image. */
	m_asset_image.load("bin/assets/images/skybox/right.jpg");
	context.m_resource_allocator.allocate_image_layered(m_image, VK_FORMAT_R8G8B8A8_SRGB,
	                                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	                                                    m_asset_image.m_width, m_asset_image.m_height, 6);
	m_asset_image.load("bin/assets/images/skybox/right.jpg");
	m_image.fill_layer(context, m_asset_image.m_data.data(), m_asset_image.m_data.size(), 0);
	m_asset_image.load("bin/assets/images/skybox/left.jpg");
	m_image.fill_layer(context, m_asset_image.m_data.data(), m_asset_image.m_data.size(), 1);
	m_asset_image.load("bin/assets/images/skybox/top.jpg");
	m_image.fill_layer(context, m_asset_image.m_data.data(), m_asset_image.m_data.size(), 2);
	m_asset_image.load("bin/assets/images/skybox/bottom.jpg");
	m_image.fill_layer(context, m_asset_image.m_data.data(), m_asset_image.m_data.size(), 3);
	m_asset_image.load("bin/assets/images/skybox/front.jpg");
	m_image.fill_layer(context, m_asset_image.m_data.data(), m_asset_image.m_data.size(), 4);
	m_asset_image.load("bin/assets/images/skybox/back.jpg");
	m_image.fill_layer(context, m_asset_image.m_data.data(), m_asset_image.m_data.size(), 5);
	m_image.transition_layout(context, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	/* Texture. */
	m_texture.build(context.m_device, m_image);

	/* Pipeline. */
	m_pipeline.add_shader(context.m_device, VK_SHADER_STAGE_VERTEX_BIT, "bin/assets/shaders/skybox.vert.spv");
	m_pipeline.add_shader(context.m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "bin/assets/shaders/skybox.frag.spv");
	m_pipeline.build(context.m_device, render_pass);
}

void skybox::draw(vulkan::command_buffer &command_buffer, vulkan::buffer &uniform_buffer) const
{
	command_buffer.bind_pipeline(m_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
	command_buffer.set_uniform_buffer(0, uniform_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
	command_buffer.set_texture(1, m_texture, VK_PIPELINE_BIND_POINT_GRAPHICS);
	vkCmdDraw(command_buffer.m_handle, 36, 1, /* firstVertex = */ 0, /* firstInstance = */ 0);
}

void static_mesh::build(vulkan::context &context, const vulkan::render_pass &render_pass, ref<assets::model> model)
{
	m_model = model;

	/* Vertex buffer. */
	context.m_resource_allocator.allocate_buffer(m_vertex_buffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                                             sizeof(m_model->m_meshes[0].m_vertices[0]) *
	                                                 m_model->m_meshes[0].m_vertices.size());
	m_vertex_buffer.fill(m_model->m_meshes[0].m_vertices.data(),
	                     sizeof(m_model->m_meshes[0].m_vertices[0]) * m_model->m_meshes[0].m_vertices.size());

	/* Index buffer. */
	context.m_resource_allocator.allocate_buffer(m_index_buffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	                                             sizeof(m_model->m_meshes[0].m_indices[0]) *
	                                                 m_model->m_meshes[0].m_indices.size());
	m_index_buffer.fill(m_model->m_meshes[0].m_indices.data(),
	                    sizeof(m_model->m_meshes[0].m_indices[0]) * m_model->m_meshes[0].m_indices.size());
	m_index_count = m_model->m_meshes[0].m_indices.size();

	/* Diffuse texture. */
	m_diffuse_image.enable_mipmaps();
	context.m_resource_allocator.allocate_image_2d(m_diffuse_image, VK_FORMAT_R8G8B8A8_SRGB,
	                                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	                                                   VK_IMAGE_USAGE_SAMPLED_BIT,
	                                               m_model->m_meshes[0].m_width, m_model->m_meshes[0].m_height);
	m_diffuse_image.fill(context, m_model->m_meshes[0].m_texture.data(), m_model->m_meshes[0].m_texture.size());
	m_diffuse_image.generate_mipmaps(context);
	m_diffuse_image.transition_layout(context, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	m_diffuse_texture.build(context.m_device, m_diffuse_image);

	/* Pipeline. */
	m_pipeline.add_shader(context.m_device, VK_SHADER_STAGE_VERTEX_BIT, "bin/assets/shaders/basic.vert.spv");
	m_pipeline.add_shader(context.m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "bin/assets/shaders/basic.frag.spv");
	m_pipeline.build(context.m_device, render_pass);
}

void static_mesh::draw(vulkan::command_buffer &command_buffer, vulkan::buffer &uniform_buffer) const
{
	command_buffer.bind_pipeline(m_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
	constexpr VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(command_buffer.m_handle, 0, 1, &m_vertex_buffer.m_handle, &offset);
	vkCmdBindIndexBuffer(command_buffer.m_handle, m_index_buffer.m_handle, 0, VK_INDEX_TYPE_UINT32);
	command_buffer.set_uniform_buffer(0, uniform_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
	command_buffer.set_texture(1, m_diffuse_texture, VK_PIPELINE_BIND_POINT_GRAPHICS);
	vkCmdDrawIndexed(command_buffer.m_handle, m_index_count,
	                 /* instanceCount = */ 1, /* firstIndex = */ 0, /* vertexOffset = */ 0,
	                 /* firstInstance = */ 0);
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
}
