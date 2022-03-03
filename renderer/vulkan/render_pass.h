#pragma once

#include <vulkan/vulkan.h>

#include <renderer/vulkan/image.h>
#include <utils/type.h>

namespace Vulkan
{

class RenderPass;

class Framebuffer
{
public:
	Framebuffer() = default;
	~Framebuffer() = default;

	Framebuffer(const Framebuffer &) = delete;
	Framebuffer operator=(const Framebuffer &) = delete;

	VkFramebuffer handle{};

	u32 width{};

	u32 height{};

	RenderPass *renderPass{};

	/* (TODO, thoave01): Assert on size when adding views? */
	void addAttachment(ImageView &attachment);

private:
	std::vector<ImageView *> attachments{};
};

class RenderPass
{
public:
	RenderPass() = default;
	~RenderPass() = default;

	RenderPass(const RenderPass &) = delete;
	RenderPass operator=(const RenderPass &) = delete;

	void build();

private:
};

} /* namespace Vulkan */
