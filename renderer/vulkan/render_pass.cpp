#include <vector>

#include <utils/util.h>

#include "render_pass.h"
#include "util.h"

namespace vulkan
{

render_pass::~render_pass()
{
	if (m_handle != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(m_device_handle, m_handle, nullptr);
	}
}

void render_pass::set_dynamic_rendering(bool dr)
{
	m_dynamic_rendering = dr;
}

void render_pass::build(device &device, VkFormat color_format, VkFormat depth_format)
{
	assert_if(!m_dynamic_rendering, "Only dynamic render passes supported");
	m_device_handle = device.m_logical.m_handle;

	m_handle = VK_NULL_HANDLE;
	m_color_format = color_format;
	m_depth_format = depth_format;
	m_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	m_rendering_info.pNext = nullptr;
	m_rendering_info.viewMask = 0;
	m_rendering_info.colorAttachmentCount = 1;
	m_rendering_info.pColorAttachmentFormats = &m_color_format;
	m_rendering_info.depthAttachmentFormat = m_depth_format;
	m_rendering_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
}

} /* namespace vulkan */
