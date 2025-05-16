#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#include <vma/vk_mem_alloc.h>
#pragma clang diagnostic pop

#include <renderer/vulkan/device.h>
#include <utils/type.h>
#include <utils/util.h>

namespace vulkan
{

class context;

struct image_info
{
	VkFormat m_format;
	u32 m_width;
	u32 m_height;
	VkImageUsageFlags m_usage;

	u32 m_layers = 1;
	bool m_mipmapped = false;
	VkSampleCountFlagBits m_sample_count = VK_SAMPLE_COUNT_1_BIT;
	VkImage m_external_image = VK_NULL_HANDLE;
};

class image
{
public:
	image() = default;
	~image();

	image(const image &) = delete;
	image operator=(const image &) = delete;

	void build(VmaAllocator allocator, const image_info &texture_info);
	void build_external(VkImage handle, VkFormat format, u32 width, u32 height);

	void transition_layout(context &context, VkImageLayout new_layout);
	void fill(context &context, const void *data, size_t size);
	void fill_layer(context &context, const void *data, size_t size, u32 layer);
	void generate_mipmaps(context &context);

	VkImage m_handle;
	image_info m_info;
	u32 m_mip_levels;
	VkImageLayout m_layout;

	/* (TODO, thoave01): Fix external. */
	bool m_external_image = false;

private:
	VmaAllocator m_allocator = VK_NULL_HANDLE;
	VmaAllocation m_allocation = VK_NULL_HANDLE;
};

class image_view
{
public:
	image_view() = default;
	~image_view();

	image_view(const image_view &) = delete;
	image_view operator=(const image_view &) = delete;

	void build(device &device, const image &image);

	VkImageView m_handle = {};
	const image *m_image = nullptr;

private:
	VkDevice m_device_handle = {};
};

class texture
{
public:
	texture() = default;
	~texture();

	texture(const texture &) = delete;
	texture operator=(const texture &) = delete;

	void build(context &context, const image_info &image_info);

	/* (TODO, thoave01): Should not be a ptr. */
	image m_image = {};
	image_view m_image_view = {};
	VkSampler m_sampler = VK_NULL_HANDLE;

private:
	VkDevice m_device_handle = {};
};

} /* namespace vulkan */
