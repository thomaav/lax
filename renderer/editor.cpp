#include <renderer/editor.h>
#include <renderer/vulkan/render_pass.h>

void editor::build_default(vulkan::context &context)
{
	/* (TODO, thoave01): Settings, this is a throwaway render pass anyway. */
	vulkan::render_pass render_pass = {};
	render_pass.set_dynamic_rendering(true);
	render_pass.build(context.m_device, m_settings.color_format, m_settings.depth_format);

	m_scene.build_default_scene(context, render_pass);
}
