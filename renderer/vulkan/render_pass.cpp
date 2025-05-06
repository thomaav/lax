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
	m_device_handle = device.m_logical.m_handle;

	if (!m_dynamic_rendering)
	{
		terminate("Only dynamic render passes supported");
	}

	m_handle = VK_NULL_HANDLE;
	m_format = format;
	m_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	m_rendering_info.pNext = nullptr;
	m_rendering_info.viewMask = 0;
	m_rendering_info.colorAttachmentCount = 1;
	m_rendering_info.pColorAttachmentFormats = &m_format;
	m_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
	m_rendering_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
}

} /* namespace vulkan */
