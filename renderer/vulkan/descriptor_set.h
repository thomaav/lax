#pragma once

#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

#include "device.h"

namespace vulkan
{

class descriptor_set_layout
{
public:
	descriptor_set_layout() = default;
	~descriptor_set_layout();

	descriptor_set_layout(const descriptor_set_layout &) = delete;
	descriptor_set_layout operator=(const descriptor_set_layout &) = delete;

	void add_binding(u32 binding, VkDescriptorType type);
	void build(device &device);

	VkDescriptorSetLayout m_handle = {};

private:
	VkDevice m_device_handle = {};
	std::vector<VkDescriptorSetLayoutBinding> m_bindings = {};
};

class descriptor_set
{
public:
	descriptor_set() = default;
	~descriptor_set();

	descriptor_set(const descriptor_set &) = delete;
	descriptor_set operator=(const descriptor_set &) = delete;

	void build(device &device);

	VkDescriptorSet m_handle = {};

private:
	VkDevice m_device_handle = {};
};

} /* namespace vulkan */
