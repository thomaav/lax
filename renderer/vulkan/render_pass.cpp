#include <vector>

#include <renderer/vulkan/render_pass.h>
#include <renderer/vulkan/util.h>
#include <utils/util.h>

namespace vulkan
{

render_pass::~render_pass()
{
	if (m_handle != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(m_device_handle, m_handle, nullptr);
	}
}

void render_pass::use_dynamic_rendering()
{
	m_dynamic_rendering = true;
}

void render_pass::build(device &device, VkFormat format)
{
	if (m_dynamic_rendering)
	{
		m_handle = VK_NULL_HANDLE;
		return;
	}

	VkAttachmentDescription color_attachment_desc = {};
	color_attachment_desc.format = format;
	color_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment_desc;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	VULKAN_ASSERT_SUCCESS(vkCreateRenderPass(device.m_logical.m_handle, &render_pass_info, nullptr, &m_handle));

	m_device_handle = device.m_logical.m_handle;
}

framebuffer::~framebuffer()
{
	if (m_handle != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(m_device_handle, m_handle, nullptr);
	}
}

void framebuffer::add_color_attachment(image_view &attachment)
{
	m_color_attachments.push_back(attachment.m_handle);

	if (m_width == 0 && m_height == 0)
	{
		m_width = attachment.m_image.m_width;
		m_height = attachment.m_image.m_height;
	}
	else
	{
		assert(m_width == attachment.m_image.m_width);
		assert(m_height == attachment.m_image.m_height);
	}
}

void framebuffer::build(device &device, render_pass &render_pass)
{
	if (m_color_attachments.size() == 0)
	{
		terminate("Attempting to build framebuffer with no attachments");
	}

	VkFramebufferCreateInfo framebuffer_info = {};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.renderPass = render_pass.m_handle;
	framebuffer_info.attachmentCount = 1;
	framebuffer_info.pAttachments = m_color_attachments.data();
	framebuffer_info.width = m_width;
	framebuffer_info.height = m_height;
	framebuffer_info.layers = 1;

	VULKAN_ASSERT_SUCCESS(vkCreateFramebuffer(device.m_logical.m_handle, &framebuffer_info, nullptr, &m_handle));

	m_device_handle = device.m_logical.m_handle;
}

} /* namespace vulkan */
