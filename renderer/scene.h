#pragma once

#include <memory>
#include <vector>

#include <assets/image.h>
#include <assets/model.h>
#include <renderer/vulkan/buffer.h>
#include <renderer/vulkan/context.h>
#include <renderer/vulkan/image.h>

template <typename T> using uref = std::unique_ptr<T>;
template <typename T, typename... Args> std::unique_ptr<T> make_uref(Args &&...args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}
template <typename T> using ref = std::shared_ptr<T>;
template <typename T, typename... Args> std::shared_ptr<T> make_ref(Args &&...args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

struct settings
{
	bool enable_mipmapping = false;
	bool enable_skybox = false;
	VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT;
};

class object
{
public:
	object() = default;
	~object() = default;

	object(const object &) = delete;
	object operator=(const object &) = delete;

private:
};

class skybox : object
{
public:
	skybox() = default;
	~skybox() = default;

	skybox(const skybox &) = delete;
	skybox operator=(const skybox &) = delete;

	void build(vulkan::context &context);

	assets::image m_asset_image = {};
	vulkan::image m_image = {};
	vulkan::texture m_texture = {};

private:
};

class static_mesh : object
{
public:
	static_mesh() = default;
	~static_mesh() = default;

	static_mesh(const static_mesh &) = delete;
	static_mesh operator=(const static_mesh &) = delete;

	void build(vulkan::context &context, ref<assets::model> model);

	ref<assets::model> m_model;
	vulkan::buffer m_vertex_buffer = {};
	vulkan::buffer m_index_buffer = {};
	vulkan::image m_diffuse_image = {}; /* (TODO, thoave01): Texture should contain image. */
	vulkan::texture m_diffuse_texture = {};

private:
};

class node
{
public:
	node() = default;
	~node() = default;

	node(const node &) = delete;
	node operator=(const node &) = delete;

private:
	std::vector<node> m_children = {};
};

class scene
{
public:
	scene() = default;
	~scene() = default;

	scene(const scene &) = delete;
	scene operator=(const scene &) = delete;

private:
	node m_root = {};
};
