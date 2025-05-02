#include <fstream>
#include <sstream>

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>
#include <simd/simd.h>

#include <model/model.h>
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
	model model = {};
	model.load("bin/models/model.obj");
	terminate("done");

	NS::Error *error = nullptr;

	/* Create queue. */
	MTL::CommandQueue *queue = m_metal_device->newCommandQueue();

	/* Create vertex buffer. */
	constexpr simd::float3 vertices[] = {
		{ -0.5f, -0.5f, 0.0f }, //
		{ 0.5f, -0.5f, 0.0f },  //
		{ 0.0f, 0.5f, 0.0f }    //
	};
	MTL::Buffer *vertex_buffer = m_metal_device->newBuffer(&vertices, sizeof(vertices), MTL::ResourceStorageModeShared);

	/* Load shaders. */
	std::string shader_code = {};
	{
		std::ifstream file("bin/shaders/metal_library.metal", std::ios::binary);
		if (!file.is_open())
		{
			terminate("Could not open shader %s", "bin/shaders/metal_library.metal");
		}

		std::stringstream file_data = {};
		file_data << file.rdbuf();
		shader_code = file_data.str();
	}
	MTL::Library *shader_library = m_metal_device->newLibrary(
	    NS::String::string(shader_code.c_str(), NS::StringEncoding::UTF8StringEncoding), nullptr, &error);
	if (nullptr != error)
	{
		terminate("%s", error->localizedDescription()->utf8String());
	}

	/* Create pipeline. */
	MTL::Function *vs =
	    shader_library->newFunction(NS::String::string("vertex_shader", NS::StringEncoding::UTF8StringEncoding));
	if (nullptr == vs)
	{
		terminate("Could not load vertex_shader function");
	}
	MTL::Function *fs =
	    shader_library->newFunction(NS::String::string("fragment_shader", NS::StringEncoding::UTF8StringEncoding));
	if (nullptr == fs)
	{
		terminate("Could not load fragment_shader function");
	}

	MTL::RenderPipelineDescriptor *ppl_desc = MTL::RenderPipelineDescriptor::alloc()->init();
	if (nullptr == ppl_desc)
	{
		terminate("Could not initialize RenderPipelineDescriptor");
	}
	ppl_desc->setVertexFunction(vs);
	ppl_desc->setFragmentFunction(fs);
	ppl_desc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
	MTL::RenderPipelineState *ppl = m_metal_device->newRenderPipelineState(ppl_desc, &error);
	if (nullptr != error)
	{
		terminate("%s", error->localizedDescription()->utf8String());
	}

	/* Create render pass. */
	MTL::RenderPassDescriptor *rpd = MTL::RenderPassDescriptor::alloc()->init();
	MTL::RenderPassColorAttachmentDescriptor *cad = rpd->colorAttachments()->object(0);
	cad->setLoadAction(MTL::LoadActionClear);
	cad->setClearColor(MTL::ClearColor(41.0f / 255.0f, 42.0f / 255.0f, 48.0f / 255.0f, 1.0));
	cad->setStoreAction(MTL::StoreActionStore);

	/* Release temporary objects used for loading. */
	ppl_desc->release();
	vs->release();
	fs->release();
	shader_library->release();

	while (m_window.step())
	{
		NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();
		{
			CA::MetalDrawable *surface = reinterpret_cast<CA::MetalDrawable *>(m_wsi.get_next_drawable());
			cad->setTexture(surface->texture());

			MTL::CommandBuffer *cmd_buf = queue->commandBuffer();
			MTL::RenderCommandEncoder *rce = cmd_buf->renderCommandEncoder(rpd);
			rce->setRenderPipelineState(ppl);
			rce->setVertexBuffer(vertex_buffer, 0, 0);
			rce->drawPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)0, (NS::UInteger)3);
			rce->endEncoding();

			cmd_buf->presentDrawable(surface);
			cmd_buf->commit();
			cmd_buf->waitUntilCompleted();
		}
		pool->release();
	}

	rpd->release();
	ppl->release();
	vertex_buffer->release();
	queue->release();
	m_metal_device->release();
}

}
