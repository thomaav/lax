#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

#include <renderer/metal/context.h>
#include <utils/util.h>

namespace metal
{

void context::build()
{
	m_window.init(800, 600);
	m_metal_device = MTL::CreateSystemDefaultDevice();
	if (nullptr == m_metal_device)
	{
		terminate("Could not create default Metal device");
	}
	m_wsi.build(m_window, m_metal_device);
}

void context::backend_test()
{
	auto commandQueue = m_metal_device->newCommandQueue();
	MTL::ClearColor color = MTL::ClearColor::Make(0, 0, 0, 1);
	while (m_window.step())
	{
		NS::AutoreleasePool *autoReleasePool = NS::AutoreleasePool::alloc()->init();
		{
			CA::MetalDrawable *surface = reinterpret_cast<CA::MetalDrawable *>(m_wsi.get_next_drawable());

			color.red = color.red > 1.0 ? 0.0 : color.red + 0.01;
			auto pass = MTL::RenderPassDescriptor::renderPassDescriptor();
			auto passColorAttachment0 = pass->colorAttachments()->object(0);
			passColorAttachment0->setClearColor(color);
			passColorAttachment0->setLoadAction(MTL::LoadActionClear);
			passColorAttachment0->setStoreAction(MTL::StoreActionStore);
			passColorAttachment0->setTexture(surface->texture());

			auto commandBuffer = commandQueue->commandBuffer();
			auto encoder = commandBuffer->renderCommandEncoder(pass);
			encoder->endEncoding();
			commandBuffer->presentDrawable(surface);
			commandBuffer->commit();
		}
		autoReleasePool->release();
	}
	m_metal_device->release();
}

}
