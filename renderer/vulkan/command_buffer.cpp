#include <renderer/vulkan/command_buffer.h>
#include <renderer/vulkan/util.h>

namespace vulkan
{

command_pool::~command_pool()
{
	if (VK_NULL_HANDLE != m_handle)
	{
		vkDestroyCommandPool(m_device_handle, m_handle, nullptr);
	}
}

void command_pool::build(device &device)
{
	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = *device.m_physical.m_queue_family.m_all;
	pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VULKAN_ASSERT_SUCCESS(vkCreateCommandPool(device.m_logical.m_handle, &pool_info, nullptr, &m_handle));

	m_device_handle = device.m_logical.m_handle;
}

void command_pool::reset()
{
	if (VK_NULL_HANDLE != m_handle)
	{
		vkResetCommandPool(m_device_handle, m_handle, 0);
	}
}

command_buffer::~command_buffer()
{
	if (VK_NULL_HANDLE != m_handle)
	{
		vkFreeCommandBuffers(m_device_handle, m_command_pool_handle, 1, &m_handle);
	}
}

void command_buffer::build(device &device, command_pool &command_pool)
{
	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = command_pool.m_handle;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = 1;

	VULKAN_ASSERT_SUCCESS(vkAllocateCommandBuffers(device.m_logical.m_handle, &alloc_info, &m_handle));

	m_device_handle = device.m_logical.m_handle;
	m_command_pool_handle = command_pool.m_handle;
}

void command_buffer::reset()
{
	if (VK_NULL_HANDLE != m_handle)
	{
		vkResetCommandBuffer(m_handle, 0);
	}
}

void command_buffer::begin()
{
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = 0;
	begin_info.pInheritanceInfo = nullptr;

	VULKAN_ASSERT_SUCCESS(vkBeginCommandBuffer(m_handle, &begin_info));
}

void command_buffer::end()
{
	VULKAN_ASSERT_SUCCESS(vkEndCommandBuffer(m_handle));
}

void command_buffer::transition_image_layout(image &image, VkImageLayout new_layout, VkPipelineStageFlagBits2 src_stage,
                                             VkAccessFlags2 src_access, VkPipelineStageFlagBits2 dst_stage,
                                             VkAccessFlags2 dst_access)
{
	const VkImageMemoryBarrier2 image_barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.pNext = nullptr,
		.srcStageMask = src_stage,
		.srcAccessMask = src_access,
		.dstStageMask = dst_stage,
		.dstAccessMask = dst_access,
		.oldLayout = image.m_layout,
		.newLayout = new_layout,
		.srcQueueFamilyIndex = 0,
		.dstQueueFamilyIndex = 0,
		.image = image.m_handle,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};
	const VkDependencyInfo dependency_info = {
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.pNext = nullptr,
		.dependencyFlags = 0,
		.memoryBarrierCount = 0,
		.pMemoryBarriers = nullptr,
		.bufferMemoryBarrierCount = 0,
		.pBufferMemoryBarriers = nullptr,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &image_barrier,
	};
	vkCmdPipelineBarrier2(m_handle, &dependency_info);
	image.m_layout = new_layout;
}

void command_buffer::bind_pipeline(const pipeline &pipeline, VkPipelineBindPoint bind_point)
{
	m_pipeline = &pipeline;
	vkCmdBindPipeline(m_handle, bind_point, pipeline.m_handle);
}

void command_buffer::set_uniform_buffer(u32 binding, const buffer &buffer, VkPipelineBindPoint bind_point)
{
	VkDescriptorBufferInfo buffer_info = {};
	buffer_info.buffer = buffer.m_handle;
	buffer_info.offset = 0;
	buffer_info.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;
	write.dstSet = 0;
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write.pBufferInfo = &buffer_info;
	vkCmdPushDescriptorSetKHR(m_handle, bind_point, m_pipeline->m_pipeline_layout->m_handle, 0, 1, &write);
}

void command_buffer::set_texture(u32 binding, const texture &texture, VkPipelineBindPoint bind_point)
{
	VkDescriptorImageInfo image_info = {};
	image_info.sampler = texture.m_sampler;
	image_info.imageView = texture.m_image_view.m_handle;
	image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;
	write.dstSet = 0;
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.pImageInfo = &image_info;
	vkCmdPushDescriptorSetKHR(m_handle, bind_point, m_pipeline->m_pipeline_layout->m_handle, 0, 1, &write);
}

} /* namespace vulkan */
