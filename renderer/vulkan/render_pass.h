#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

#include <renderer/vulkan/device.h>
#include <renderer/vulkan/image.h>
#include <renderer/vulkan/render_pass.h>
#include <utils/type.h>

namespace vulkan
{

class render_pass
{
public:
	render_pass() = default;
	~render_pass();

	render_pass(const render_pass &) = delete;
	render_pass operator=(const render_pass &) = delete;

	void use_dynamic_rendering();
	void build(device &device, VkFormat format);

	VkRenderPass m_handle = {};

private:
	VkDevice m_device_handle = {};
	bool m_dynamic_rendering = false;
};

class framebuffer
{
public:
	framebuffer() = default;
	~framebuffer();

	framebuffer(const framebuffer &) = delete;
	framebuffer operator=(const framebuffer &) = delete;

	VkFramebuffer m_handle = {};

	void add_color_attachment(image_view &attachment);
	void build(device &device, render_pass &render_pass);

private:
	std::vector<VkImageView> m_color_attachments = {};
	u32 m_width = 0;
	u32 m_height = 0;
	VkDevice m_device_handle = {};
};

} /* namespace vulkan */
