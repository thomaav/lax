#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

#include <renderer/vulkan/device.h>

namespace vulkan
{

class image
{
public:
	image() = default;
	~image() = default;

	image(const image &) = delete;
	image operator=(const image &) = delete;

	VkImage m_handle = {};
	VkFormat m_format = {};
	u32 m_width = {};
	u32 m_height = {};

private:
};

class image_view
{
public:
	image_view(image &image);
	~image_view();

	image_view(const image_view &) = delete;
	image_view operator=(const image_view &) = delete;

	void build(device &device);

	VkImageView m_handle = {};

	VkDevice m_device_handle = {};
	const image &m_image = {};

private:
};

} /* namespace vulkan */
