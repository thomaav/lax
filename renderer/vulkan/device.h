#pragma once

namespace Vulkan
{

class Device
{
public:
	Device();
	~Device();

	Device(const Device &) = delete;
	Device operator=(const Device &) = delete;

private:
	;
};

}
