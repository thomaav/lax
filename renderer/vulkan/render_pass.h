#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

#include <utils/type.h>

#include "device.h"
#include "image.h"
#include "render_pass.h"

namespace vulkan
{

class render_pass
{
public:
	render_pass() = default;
	~render_pass();

	render_pass(const render_pass &) = delete;
	render_pass operator=(const render_pass &) = delete;

	void set_dynamic_rendering(bool dr);
	void build(device &device, VkFormat color_format, VkFormat depth_format);

	VkRenderPass m_handle = {};
	VkFormat m_color_format = VK_FORMAT_UNDEFINED;
	VkFormat m_depth_format = VK_FORMAT_UNDEFINED;
	VkPipelineRenderingCreateInfo m_rendering_info = {};

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
