#include <vector>

#include <renderer/vulkan/render_pass.h>
#include <renderer/vulkan/util.h>
#include <utils/util.h>

namespace Vulkan
{

RenderPass::~RenderPass()
{
	if (handle != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(device, handle, nullptr);
	}
}

void RenderPass::build(Device &device, VkFormat format)
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VULKAN_ASSERT_SUCCESS(vkCreateRenderPass(device.logical.handle, &renderPassInfo, nullptr, &handle));

	this->device = device.logical.handle;
}

Framebuffer::~Framebuffer()
{
	if (handle != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(device, handle, nullptr);
	}
}

void Framebuffer::addColorAttachment(ImageView &attachment)
{
	colorAttachments.push_back(attachment.handle);

	if (width == 0 && height == 0)
	{
		width = attachment.image.width;
		height = attachment.image.height;
	}
	else
	{
		assert(width == attachment.image.width);
		assert(height == attachment.image.height);
	}
}

void Framebuffer::build(Device &device, RenderPass &renderPass)
{
	if (colorAttachments.size() == 0)
	{
		terminate("Attempting to build framebuffer with no attachments");
	}

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass.handle;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = colorAttachments.data();
	framebufferInfo.width = width;
	framebufferInfo.height = height;
	framebufferInfo.layers = 1;

	VULKAN_ASSERT_SUCCESS(vkCreateFramebuffer(device.logical.handle, &framebufferInfo, nullptr, &handle));

	this->device = device.logical.handle;
}

} /* namespace Vulkan */
