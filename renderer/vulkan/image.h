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

	image(image &&o) noexcept;
	image &operator=(image &&o) noexcept;

	void set_mipmaps(bool mipmaps);
	void set_sample_count(VkSampleCountFlagBits sample_count);

	void build_2d(VmaAllocator allocator, VkFormat format, VkImageUsageFlags usage, u32 width, u32 height);
	void build_layered(VmaAllocator allocator, VkFormat format, VkImageUsageFlags usage, u32 width, u32 height,
	                   u32 layers);
	void build_external(VkImage handle, VkFormat format, u32 width, u32 height);

	void transition_layout(context &context, VkImageLayout new_layout);
	void fill(context &context, const void *data, size_t size);
	void fill_layer(context &context, const void *data, size_t size, u32 layer);
	void generate_mipmaps(context &context);

	bool m_external_image = false;
	VkImage m_handle = {};
	VkFormat m_format = {};
	u32 m_width = 0;
	u32 m_height = 0;
	u32 m_layers = 0;
	bool m_mipmapped = false;
	VkSampleCountFlagBits m_sample_count = VK_SAMPLE_COUNT_1_BIT;
	u32 m_mip_levels = 1;
	VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;

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

	image_view(image_view &&o) noexcept;
	image_view &operator=(image_view &&o) noexcept;

	void build(device &device, const ref<image> &image);

	VkImageView m_handle = {};
	ref<image> m_image = nullptr;

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

	texture(texture &&o) noexcept;
	texture &operator=(texture &&o) noexcept;

	void build(device &device, const ref<image> &image);

	ref<image> m_image = nullptr;
	image_view m_image_view = {};
	VkSampler m_sampler = VK_NULL_HANDLE;

private:
	VkDevice m_device_handle = {};
};

} /* namespace vulkan */
