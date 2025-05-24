#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <third_party/volk/volk.h>
#pragma clang diagnostic pop

#include "buffer.h"
#include "device.h"
#include "image.h"
#include "pipeline.h"

namespace vulkan
{

class command_pool
{
public:
	command_pool() = default;
	~command_pool();

	command_pool(const command_pool &) = delete;
	command_pool operator=(const command_pool &) = delete;

	void build(device &device);
	void reset();

	VkCommandPool m_handle = {};

private:
	VkDevice m_device_handle = {};
};

class command_buffer
{
public:
	command_buffer() = default;
	~command_buffer();

	command_buffer(const command_buffer &) = delete;
	command_buffer operator=(const command_buffer &) = delete;

	void build(device &device, command_pool &command_pool);
	void reset();
	void begin();
	void end();
	void transition_image_layout(image &image, VkImageLayout new_layout, VkPipelineStageFlagBits2 src_stage,
	                             VkAccessFlags2 src_access, VkPipelineStageFlagBits2 dst_stage,
	                             VkAccessFlags2 dst_access);
	void bind_pipeline(const pipeline &pipeline, VkPipelineBindPoint bind_point);
	void set_uniform_buffer(u32 binding, const buffer &buffer, VkPipelineBindPoint bind_point);
	void set_texture(u32 binding, const texture &texture, VkPipelineBindPoint bind_point);

	VkCommandBuffer m_handle = {};

private:
	VkDevice m_device_handle = {};
	VkCommandPool m_command_pool_handle = {};
	const pipeline *m_pipeline = nullptr;
};

} /* namespace vulkan */
