#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <model/model.h>
#include <utils/util.h>

void model::load(const char *path)
{
	Assimp::Importer importer = {};
	const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate);
	if (nullptr == scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || nullptr == scene->mRootNode)
	{
		terminate("Assimp could not load %s", path);
	}

	m_meshes.reserve(scene->mNumMeshes);
	for (u32 mesh_idx = 0; mesh_idx < scene->mNumMeshes; ++mesh_idx)
	{
		const aiMesh *assimp_mesh = scene->mMeshes[mesh_idx];
		mesh &mesh = m_meshes[mesh_idx];

		/* Add vertices. */
		for (u32 v_idx = 0; v_idx < assimp_mesh->mNumVertices; ++v_idx)
		{
			const aiVector3D &pos = assimp_mesh->mVertices[v_idx];
			const vertex vertex = {
				.position = { pos.x, pos.y, pos.z },
				.normal = {},
				.uv = {},
			};
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
	}
}
