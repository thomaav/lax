#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#include <vma/vk_mem_alloc.h>
#pragma clang diagnostic pop

namespace vulkan
{

/* Forward declarations. */
class buffer;
class context;

class image
{
public:
	image() = default;
	~image();

	image(const image &) = delete;
	image operator=(const image &) = delete;

	void build(VmaAllocator allocator, VkFormat format, VkImageUsageFlags usage, u32 width, u32 height);
	void build_external_image(VkImage handle, VkFormat format, u32 width, u32 height);
	void transition_layout(context &context, VkImageLayout old_layout, VkImageLayout new_layout);
	void fill(context &context, const void *data, size_t size);

	bool m_external_image = false;
	VkImage m_handle = {};
	VkFormat m_format = {};
	u32 m_width = {};
	u32 m_height = {};
	VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;

private:
	VmaAllocator m_allocator = VK_NULL_HANDLE;
	VmaAllocation m_allocation = VK_NULL_HANDLE;
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
