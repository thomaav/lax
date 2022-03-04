#pragma once

#include <vulkan/vulkan.h>

#include <renderer/vulkan/device.h>
#include <renderer/vulkan/image.h>
#include <renderer/vulkan/render_pass.h>
#include <utils/type.h>

namespace Vulkan
{

class RenderPass
{
public:
	RenderPass() = default;
	~RenderPass();

	RenderPass(const RenderPass &) = delete;
	RenderPass operator=(const RenderPass &) = delete;

	void build(Device &device, VkFormat format);

	VkRenderPass handle{};

private:
	VkDevice device{};
};

class Framebuffer
{
public:
	Framebuffer() = default;
	~Framebuffer();

	Framebuffer(const Framebuffer &) = delete;
	Framebuffer operator=(const Framebuffer &) = delete;

	VkFramebuffer handle{};

	/* (TODO, thoave01): Assert on size when adding views? */
	void addColorAttachment(ImageView &attachment);

	void build(Device &device, RenderPass &renderPass);

private:
	std::vector<VkImageView> colorAttachments{};

	u32 width{};

	u32 height{};

	VkDevice device{};
};

} /* namespace Vulkan */
