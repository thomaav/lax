#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/gtc/type_ptr.hpp>
#include <third_party/stb/stb_image.h>
#pragma clang diagnostic pop

#include <utils/log.h>
#include <utils/type.h>
#include <utils/util.h>

#include "model.h"

const aiNode *find_mesh_node(const aiScene *scene, const aiNode *node, const aiMesh *mesh)
{
	for (u32 i = 0; i < node->mNumMeshes; ++i)
	{
		if (scene->mMeshes[node->mMeshes[i]] == mesh)
		{
			return node;
		}
	}

	for (u32 i = 0; i < node->mNumChildren; ++i)
	{
		const aiNode *found = find_mesh_node(scene, node->mChildren[i], mesh);
		if (found)
		{
			return found;
		}
	}

	return nullptr;
}

namespace assets
{

void model::load(const char *path)
{
	Assimp::Importer importer = {};
	const aiScene *scene = importer.ReadFile(path, aiProcess_FlipUVs);
	assert_if(nullptr == scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || nullptr == scene->mRootNode,
	          "Assimp could not load %s", path);

	/* Parse the mesh. */
	m_meshes.reserve(scene->mNumMeshes);
	for (u32 mesh_idx = 0; mesh_idx < scene->mNumMeshes; ++mesh_idx)
	{
		const aiMesh *assimp_mesh = scene->mMeshes[mesh_idx];
		const aiMaterial *assimp_material = scene->mMaterials[assimp_mesh->mMaterialIndex];

		m_meshes.push_back({});
		mesh &mesh = m_meshes[mesh_idx];

		/* Add vertices. */
		for (u32 v_idx = 0; v_idx < assimp_mesh->mNumVertices; ++v_idx)
		{
			vertex vertex = {};

			/* Get position. */
			const aiVector3D &pos = assimp_mesh->mVertices[v_idx];
			vertex.position = { pos.x, pos.y, pos.z };

			/* Get normal. */
			if (assimp_mesh->HasNormals())
			{
				const aiVector3D &normal = assimp_mesh->mNormals[v_idx];
				vertex.normal = { normal.x, normal.y, normal.z };
			}

			/* Get UV. */
			if (assimp_mesh->HasTextureCoords(0))
			{
				const aiVector3D &uv = assimp_mesh->mTextureCoords[0][v_idx];
				vertex.uv = { uv.x, uv.y };
			}

			/* Get color. */
			if (assimp_mesh->HasVertexColors(0))
			{
				const aiColor4D &color = assimp_mesh->mColors[0][v_idx];
				vertex.color = { color.r, color.g, color.b, color.a };
			}
			else
			{
				float random_color = random_float();
				vertex.color = { random_color, random_color, random_color, 1.0f };
			}

			mesh.m_vertices.push_back(vertex);
		}

		/* Add indices. */
		for (u32 f_idx = 0; f_idx < assimp_mesh->mNumFaces; ++f_idx)
		{
			const aiFace &face = assimp_mesh->mFaces[f_idx];
			for (u32 i_idx = 0; i_idx < face.mNumIndices; ++i_idx)
			{
				mesh.m_indices.push_back(face.mIndices[i_idx]);
			}
		}

		/* Add material. */
		constexpr u32 texture_idx = 0;
		aiString texture_path = {};
		if (AI_SUCCESS == assimp_material->GetTexture(aiTextureType_DIFFUSE, texture_idx, &texture_path))
		{
			/* Validate texture. */
			aiTexture *texture = scene->mTextures[texture_idx];
			assert_if(nullptr == texture, "Assimp could not get diffuse aiTexture for %s, mesh %u", path, mesh_idx);
			assert_if(texture->mHeight != 0, "Found raw texture data with Assimp, handling not implemented");

			/* Decode texture data. */
			int channels;
			stbi_uc *texture_data = stbi_load_from_memory((u8 *)texture->pcData, /* len = */ texture->mWidth,
			                                              &mesh.m_width, &mesh.m_height, &channels, STBI_rgb_alpha);
			assert_if(nullptr == texture_data, "stbi_load_from_memory could not load texture for model %s", path);
			mesh.m_texture.assign(texture_data, texture_data + mesh.m_width * mesh.m_height * 4);
			stbi_image_free(texture_data);
		}
		else
		{
			assert_if(true, "Assimp could not get diffuse texture for %s, mesh %u", path, mesh_idx);
		}

		/* Add transform. */
		const aiNode *mesh_node = find_mesh_node(scene, scene->mRootNode, assimp_mesh);
		assert_if(nullptr == mesh_node, "Could not find mesh %u scene node in model %s", mesh_idx, path);
		aiMatrix4x4 transform = mesh_node->mTransformation;
		const aiNode *node = mesh_node->mParent;
		while (nullptr != node)
		{
			transform = node->mTransformation * transform;
			node = node->mParent;
		}
		mesh.m_transform = glm::transpose(glm::make_mat4(&transform.a1));
	}
}

void model::generate_grid()
{
	int grid_min = -64.0f;
	int grid_max = 64.0f;
	float line_width = 0.01f;
	glm::vec4 color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

	m_meshes.push_back({});
	mesh &mesh = m_meshes[0];

	for (int x = grid_min; x <= grid_max; ++x)
	{
		glm::vec3 v1 = { (float)x - line_width, 0.0f, (float)grid_min };
		glm::vec3 v2 = { (float)x - line_width, 0.0f, (float)grid_max };
		glm::vec3 v3 = { (float)x + line_width, 0.0f, (float)grid_min };
		glm::vec3 v4 = { (float)x + line_width, 0.0f, (float)grid_max };
		mesh.m_vertices.push_back({ v1, /* normal = */ {}, /* texture = */ {}, color });
		mesh.m_vertices.push_back({ v2, /* normal = */ {}, /* texture = */ {}, color });
		mesh.m_vertices.push_back({ v3, /* normal = */ {}, /* texture = */ {}, color });
		mesh.m_vertices.push_back({ v2, /* normal = */ {}, /* texture = */ {}, color });
		mesh.m_vertices.push_back({ v3, /* normal = */ {}, /* texture = */ {}, color });
		mesh.m_vertices.push_back({ v4, /* normal = */ {}, /* texture = */ {}, color });
	}
	for (int z = grid_min; z <= grid_max; ++z)
	{
		glm::vec3 v1 = { (float)grid_min, 0.0f, (float)z - line_width };
		glm::vec3 v2 = { (float)grid_max, 0.0f, (float)z - line_width };
		glm::vec3 v3 = { (float)grid_min, 0.0f, (float)z + line_width };
		glm::vec3 v4 = { (float)grid_max, 0.0f, (float)z + line_width };
		mesh.m_vertices.push_back({ v1, /* normal = */ {}, /* texture = */ {}, color });
		mesh.m_vertices.push_back({ v2, /* normal = */ {}, /* texture = */ {}, color });
		mesh.m_vertices.push_back({ v3, /* normal = */ {}, /* texture = */ {}, color });
		mesh.m_vertices.push_back({ v2, /* normal = */ {}, /* texture = */ {}, color });
		mesh.m_vertices.push_back({ v3, /* normal = */ {}, /* texture = */ {}, color });
		mesh.m_vertices.push_back({ v4, /* normal = */ {}, /* texture = */ {}, color });
	}
}

void model::generate_plane()
{
	int plane_min = -64.0f;
	int plane_max = 64.0f;

	m_meshes.push_back({});
	mesh &mesh = m_meshes[0];

	glm::vec3 v1 = { (float)plane_min, -0.003f, (float)plane_min };
	glm::vec3 v2 = { (float)plane_min, -0.003f, (float)plane_max };
	glm::vec3 v3 = { (float)plane_max, -0.003f, (float)plane_min };
	glm::vec3 v4 = { (float)plane_max, -0.003f, (float)plane_max };
	mesh.m_vertices.push_back({ v1, /* normal = */ {}, /* texture = */ {}, /* color = */ {} });
	mesh.m_vertices.push_back({ v2, /* normal = */ {}, /* texture = */ {}, /* color = */ {} });
	mesh.m_vertices.push_back({ v3, /* normal = */ {}, /* texture = */ {}, /* color = */ {} });
	mesh.m_vertices.push_back({ v2, /* normal = */ {}, /* texture = */ {}, /* color = */ {} });
	mesh.m_vertices.push_back({ v3, /* normal = */ {}, /* texture = */ {}, /* color = */ {} });
	mesh.m_vertices.push_back({ v4, /* normal = */ {}, /* texture = */ {}, /* color = */ {} });
}

} /* namespace assets */
