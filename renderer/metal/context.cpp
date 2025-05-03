#include <chrono>
#include <fstream>
#include <sstream>

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>
#include <simd/simd.h>

#include <model/model.h>
#include <renderer/metal/context.h>
#include <utils/util.h>

struct uniforms
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};
static_assert(sizeof(uniforms) == 4 * 4 * 4 * 3, "Unexpected struct uniform size");

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
	model.load("bin/assets/models/DamagedHelmet.glb");

	NS::Error *error = nullptr;

	/* Create queue. */
	MTL::CommandQueue *queue = m_metal_device->newCommandQueue();

	/* Create vertex buffer. */
	MTL::Buffer *vertex_buffer = m_metal_device->newBuffer(
	    model.m_meshes[0].m_vertices.data(),
	    sizeof(model.m_meshes[0].m_vertices[0]) * model.m_meshes[0].m_vertices.size(), MTL::ResourceStorageModeShared);
	MTL::Buffer *index_buffer = m_metal_device->newBuffer(
	    model.m_meshes[0].m_indices.data(), sizeof(model.m_meshes[0].m_indices[0]) * model.m_meshes[0].m_indices.size(),
	    MTL::ResourceStorageModeShared);

	/* Create uniforms. */
	uniforms uniforms = {};
	uniforms.model = glm::mat4(1.0f);
	uniforms.view = glm::lookAt(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	uniforms.projection = glm::perspective(glm::radians(45.0f), 800 / (float)600, 0.1f, 10.0f);
	uniforms.projection[1][1] *= -1.0f;
	MTL::Buffer *uniform_buffer =
	    m_metal_device->newBuffer(&uniforms, sizeof(uniforms), MTL::ResourceStorageModeShared);

	/* Create texture. */
	MTL::TextureDescriptor *td = MTL::TextureDescriptor::alloc()->init();
	td->setWidth(model.m_meshes[0].m_width);
	td->setHeight(model.m_meshes[0].m_height);
	td->setPixelFormat(MTL::PixelFormatRGBA8Unorm_sRGB);
	td->setTextureType(MTL::TextureType2D);
	td->setStorageMode(MTL::StorageModeShared);
	td->setUsage(MTL::ResourceUsageSample | MTL::ResourceUsageRead);
	MTL::Texture *texture = m_metal_device->newTexture(td);
	texture->replaceRegion(MTL::Region(0, 0, 0, model.m_meshes[0].m_width, model.m_meshes[0].m_height, 1), 0,
	                       model.m_meshes[0].m_texture.data(), model.m_meshes[0].m_width * 4);
	td->release();

	/* Create depth texture. */
	MTL::TextureDescriptor *dtd = MTL::TextureDescriptor::alloc()->init();
	dtd->setTextureType(MTL::TextureType2D);
	dtd->setPixelFormat(MTL::PixelFormatDepth16Unorm);
	dtd->setWidth(800);
	dtd->setHeight(600);
	dtd->setUsage(MTL::TextureUsageRenderTarget);
	MTL::Texture *depth_texture = m_metal_device->newTexture(dtd);
	dtd->release();

	/* Load shaders. */
	std::string shader_code = {};
	{
		std::ifstream file("bin/assets/shaders/metal_library.metal", std::ios::binary);
		if (!file.is_open())
		{
			terminate("Could not open shader %s", "bin/assets/shaders/metal_library.metal");
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
	ppl_desc->setDepthAttachmentPixelFormat(MTL::PixelFormat::PixelFormatDepth16Unorm);
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
	MTL::RenderPassDepthAttachmentDescriptor *dad = rpd->depthAttachment();
	dad->setTexture(depth_texture);
	dad->setLoadAction(MTL::LoadActionClear);
	dad->setStoreAction(MTL::StoreActionDontCare);
	dad->setClearDepth(1.0);

	/* Create depth/stencil state. */
	MTL::DepthStencilDescriptor *dsd = MTL::DepthStencilDescriptor::alloc()->init();
	dsd->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
	dsd->setDepthWriteEnabled(true);
	MTL::DepthStencilState *ds_state = m_metal_device->newDepthStencilState(dsd);
	dsd->release();

	/* Release temporary objects used for loading. */
	ppl_desc->release();
	vs->release();
	fs->release();
	shader_library->release();

	while (m_window.step())
	{
		static auto start_time = std::chrono::high_resolution_clock::now();
		auto current_time = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
		struct uniforms *uniforms_ptr = reinterpret_cast<struct uniforms *>(uniform_buffer->contents());
		uniforms_ptr->model = glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();
		{
			CA::MetalDrawable *surface = reinterpret_cast<CA::MetalDrawable *>(m_wsi.get_next_drawable());
			rpd->colorAttachments()->object(0)->setTexture(surface->texture());

			MTL::CommandBuffer *cmd_buf = queue->commandBuffer();
			MTL::RenderCommandEncoder *rce = cmd_buf->renderCommandEncoder(rpd);
			rce->setRenderPipelineState(ppl);
			rce->setDepthStencilState(ds_state);
			rce->setVertexBuffer(vertex_buffer, /* offset = */ 0, /* index = */ 0);
			rce->setVertexBuffer(uniform_buffer, /* offset = */ 0, /* index = */ 1);
			rce->setFragmentTexture(texture, /* index 0 = */ 0);
			rce->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, model.m_meshes[0].m_indices.size(),
			                           MTL::IndexTypeUInt32, index_buffer,
			                           /* indexBufferOffset = */ (NS::UInteger)0);
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
	index_buffer->release();
	texture->release();
	depth_texture->release();
	ds_state->release();
	queue->release();
	m_metal_device->release();
}
}
