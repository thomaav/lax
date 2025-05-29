#include "descriptor_set.h"
#include "util.h"

namespace vulkan
{

descriptor_set_layout::~descriptor_set_layout()
{
	if (VK_NULL_HANDLE != m_handle)
	{
		vkDestroyDescriptorSetLayout(m_device_handle, m_handle, nullptr);
	}
}

void descriptor_set_layout::add_binding(u32 binding, VkDescriptorType type)
{
	VkDescriptorSetLayoutBinding dsl_binding = {};
	dsl_binding.binding = binding;
	dsl_binding.descriptorType = type;
	dsl_binding.descriptorCount = 1;              /* (TODO, thoave01): No arrays. */
	dsl_binding.stageFlags = VK_SHADER_STAGE_ALL; /* (TODO, thoave01): What's the overhead? */
	dsl_binding.pImmutableSamplers = nullptr;
	m_bindings.push_back(dsl_binding);
}

void descriptor_set_layout::build(device &device)
{
	m_device_handle = device.m_logical.m_handle;
	VkDescriptorSetLayoutCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,     //
		.pNext = nullptr,                                                 //
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR, //
		.bindingCount = (u32)m_bindings.size(),                           //
		.pBindings = m_bindings.data(),                                   //
	};
	VULKAN_ASSERT_SUCCESS(vkCreateDescriptorSetLayout(m_device_handle, &create_info, nullptr, &m_handle));
}

descriptor_set::~descriptor_set()
{
}

void descriptor_set::build(device &device)
{
	m_device_handle = device.m_logical.m_handle;
}

} /* namespace vulkan */
