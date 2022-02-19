#include <renderer/vulkan/queue.h>

namespace Vulkan
{

void Queue::build(Device &device)
{
	/* (TODO, thoave01): Split presentation and graphics queue? Etc. */
	vkGetDeviceQueue(device.logical.handle, *device.physical.queueFamily.all, 0, &handle);
}

} /* namespace Vulkan */
