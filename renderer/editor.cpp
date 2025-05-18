#include <renderer/editor.h>
#include <renderer/vulkan/render_pass.h>

/* (TODO, thoave01): Move width and stuff into editor settings. */
constexpr u32 WINDOW_WIDTH = 1280;
constexpr u32 WINDOW_HEIGHT = 900;

void editor::build_default(vulkan::context &context)
{
	/* (TODO, thoave01): Settings, this is a throwaway render pass anyway. */
	vulkan::render_pass render_pass = {};
	render_pass.set_dynamic_rendering(true);
	render_pass.build(context.m_device, m_settings.color_format, m_settings.depth_format);

	m_scene.build_default_scene(context, render_pass);
}

void editor::draw(vulkan::command_buffer &command_buffer)
{
	/* (TODO, thoave01): Camera object. */
	m_scene.m_uniforms.view =
	    glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_scene.m_uniforms.projection =
	    glm::perspectiveRH_ZO(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 256.0f);
	m_scene.m_uniforms.enable_mipmapping = m_settings.enable_mipmapping;
	m_scene.m_uniform_buffer.fill(&m_scene.m_uniforms, sizeof(m_scene.m_uniforms));

	/* (TODO, thoave01): Add some sort of default pipeline with scene defaults and pipeline compatibility. */
	command_buffer.bind_pipeline(m_scene.m_skybox->m_pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
	command_buffer.set_uniform_buffer(0, m_scene.m_uniform_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);

	m_scene.m_root.m_children[0].m_object->draw(command_buffer);
	if (m_settings.enable_skybox)
	{
		m_scene.m_root.m_children[1].m_object->draw(command_buffer);
	}
}
