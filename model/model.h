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

class mesh
{
public:
	mesh() = default;
	~mesh() = default;

	mesh(const mesh &) = default;
	mesh operator=(const mesh &) = delete;

	std::vector<vertex> m_vertices = {};
	std::vector<u32> m_indices = {};

	std::vector<unsigned char> m_texture = {};
	int m_width, m_height;

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
