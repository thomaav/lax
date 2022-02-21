#pragma once

#include <vulkan/vulkan.h>

#include <renderer/vulkan/device.h>

namespace Vulkan
{

class Image
{
public:
	Image() = default;
	~Image() = default;

	Image(const Image &) = delete;
	Image operator=(const Image &) = delete;

	VkImage handle{};

	VkFormat format{};

	u32 width{};

	u32 height{};

	/* (TODO, thoave01): Actually initialize images this way. */

private:
};

class ImageView
{
public:
	ImageView(Image &image);
	~ImageView() = default;

	ImageView(const ImageView &) = delete;
	ImageView operator=(const ImageView &) = delete;

	void build(Device &device);

	void destroy();

	VkImageView handle{};

	VkDevice device{};

	const Image &image{};

private:
};

} /* namespace Vulkan */
