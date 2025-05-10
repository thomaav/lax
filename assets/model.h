#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <utils/type.h>

struct vertex
{
	glm::vec3 position = {};
	glm::vec3 normal = {};
	glm::vec2 uv = {};
	glm::vec4 color = {};
};
static_assert(sizeof(vertex) == 48, "Unexpected struct vertex size");

namespace assets
{

class mesh
{
public:
	mesh() = default;
	~mesh() = default;

	mesh(const mesh &) = default;
	mesh operator=(const mesh &) = delete;

	std::vector<vertex> m_vertices = {};
	std::vector<u32> m_indices = {};

	std::vector<u8> m_texture = {};
	int m_width = -1;
	int m_height = -1;
	glm::mat4 m_transform = {};

private:
};

class model
{
public:
	model() = default;
	~model() = default;

	model(const model &) = delete;
	model operator=(const model &) = delete;

	void load(const char *path);

	std::vector<mesh> m_meshes;

private:
};

} /* namespace assets */
