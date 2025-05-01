#pragma once

#include <platform/window.h>
#include <renderer/metal/wsi.h>

namespace metal
{

class context
{
public:
	context() = default;
	~context() = default;

	context(const context &) = delete;
	context operator=(const context &) = delete;

	void build();
	void backend_test();

private:
	glfw_window m_window = {};
	wsi m_wsi = {};
	MTL::Device *m_metal_device = nullptr; /* (TODO, thoave01): Class. */
};

}
