#pragma once

#include <Metal/Metal.hpp>

#include <platform/window.h>

namespace metal
{

class wsi
{
public:
	wsi() = default;
	~wsi() = default;

	wsi(const wsi &) = delete;
	wsi operator=(const wsi &) = delete;

	void build(glfw_window &window, MTL::Device *metal_device);
	id get_next_drawable();

private:
	id m_ns_window = nullptr;
	id m_metal_layer = nullptr;
};

}
