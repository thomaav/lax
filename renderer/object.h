#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// clang-format on

#include <assets/image.h>
#include <assets/model.h>
#include <renderer/vulkan/buffer.h>
#include <renderer/vulkan/command_buffer.h>

/* (TODO, thoave01): Move width and stuff into editor settings. */
constexpr u32 WINDOW_WIDTH = 1280;
constexpr u32 WINDOW_HEIGHT = 900;

struct object_uniforms
{
	glm::mat4 model;
};
static_assert(sizeof(object_uniforms) == 4 * 4 * 4, "Unexpected object struct uniform size");

class object
{
public:
	object() = default;
	virtual ~object() = default;

	virtual void draw(vulkan::command_buffer &command_buffer) = 0;

private:
};

class skybox : public object
{
public:
	skybox() = default;
	~skybox() = default;

	skybox(const skybox &) = delete;
	skybox operator=(const skybox &) = delete;

	void build(vulkan::context &context, const vulkan::render_pass &render_pass);
	void draw(vulkan::command_buffer &command_buffer) override;
	void update_material(const vulkan::render_pass &render_pass, VkSampleCountFlagBits sample_count);

	assets::image m_asset_image = {};
	vulkan::texture m_texture = {};
	vulkan::pipeline m_pipeline = {};
	object_uniforms m_uniforms = {};

private:
};

class static_mesh : public object
{
public:
	static_mesh() = default;
	~static_mesh() = default;

	static_mesh(const static_mesh &) = delete;
	static_mesh operator=(const static_mesh &) = delete;

	void build(vulkan::context &context, const vulkan::render_pass &render_pass, ref<assets::model> model);
	void draw(vulkan::command_buffer &command_buffer) override;
	void update_material(const vulkan::render_pass &render_pass, VkSampleCountFlagBits sample_count);

	ref<assets::model> m_model;
	vulkan::buffer m_vertex_buffer = {};
	u32 m_index_count = 0;
	vulkan::buffer m_index_buffer = {};
	vulkan::texture m_diffuse_texture = {};
	vulkan::pipeline m_pipeline = {};
	object_uniforms m_uniforms = {};

private:
};

class camera : public object
{
public:
	camera() = default;
	~camera() = default;

	camera(const camera &) = delete;
	camera operator=(const camera &) = delete;

	void draw(vulkan::command_buffer &command_buffer) override;

	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 projection =
	    glm::perspectiveRH_ZO(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 256.0f);

private:
};
