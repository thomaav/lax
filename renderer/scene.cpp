#include <renderer/scene.h>

void skybox::build(vulkan::context &context)
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
}

void static_mesh::build(vulkan::context &context, ref<assets::model> model)
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
}
