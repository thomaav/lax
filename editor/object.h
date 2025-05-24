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

struct object_uniforms
{
	glm::mat4 model;
};
static_assert(sizeof(object_uniforms) == 4 * 4 * 4, "Unexpected object struct uniform size");

class object
{
public:
	virtual ~object() = default;
	virtual void draw(vulkan::command_buffer &command_buffer) = 0;
};

class skybox : public object
{
public:
	skybox() = default;
	~skybox() = default;

	skybox(const skybox &) = delete;
	skybox operator=(const skybox &) = delete;

	void build(vulkan::context &context);
	void draw(vulkan::command_buffer &command_buffer) override;
	void update_material(VkSampleCountFlagBits sample_count);

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

	void build(vulkan::context &context, ref<assets::model> model);
	void draw(vulkan::command_buffer &command_buffer) override;
	void update_material(VkSampleCountFlagBits sample_count);

	ref<assets::model> m_model;
	vulkan::buffer m_vertex_buffer = {};
	u32 m_index_count = 0;
	vulkan::buffer m_index_buffer = {};
	vulkan::texture m_diffuse_texture = {};
	vulkan::pipeline m_pipeline = {};
	object_uniforms m_uniforms = {};

private:
};

struct camera_input
{
	bool w_pressed = false;
	bool a_pressed = false;
	bool s_pressed = false;
	bool d_pressed = false;
	double mouse_x = 0.0;
	double mouse_y = 0.0;
	bool right_mouse_pressed = false;
	bool middle_mouse_pressed = false;
};

class camera : public object
{
public:
	camera() = default;
	~camera() = default;

	camera(const camera &) = delete;
	camera operator=(const camera &) = delete;

	void build(glm::vec3 position, glm::vec3 target);
	void process_input(const camera_input &input);
	void draw(vulkan::command_buffer &command_buffer) override;

	/* Depends on viewport size. */
	float m_aspect;

	/* Needed by rendering. */
	glm::mat4 m_view;
	glm::mat4 m_projection;

private:
	const float m_speed = 1.0f;
	const float m_sensitivity = 6.0f;

	glm::vec3 m_position;
	glm::vec3 m_forward;

	float m_yaw;
	float m_pitch;
	glm::vec3 m_up;

	float m_fov;
	float m_near;
	float m_far;
};
