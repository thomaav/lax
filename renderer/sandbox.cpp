#include <renderer/vulkan/command_buffer.h>
#include <renderer/vulkan/context.h>
#include <utils/util.h>

#include "render_graph.h"
#include "sandbox.h"

void run_sandbox()
{
	vulkan::context context = {};
	context.build();

	render_graph rg(context);
	rg.reset();

	render_pass &rp1 = rg.add_render_pass("rp1");
	rp1.add_buffer("constants1");
	rp1.add_texture("color1");
	rp1.set_execution(
	    [&](vulkan::command_buffer &cmd_buf)
	    {
		    UNUSED(cmd_buf);
		    printf("rp1\n");
	    });

	render_pass &rp2 = rg.add_render_pass("rp2");
	rp2.add_buffer("constants2");
	rp2.add_texture("color1");
	rp2.add_texture("color2");
	rp2.set_execution(
	    [&](vulkan::command_buffer &cmd_buf)
	    {
		    UNUSED(cmd_buf);
		    printf("rp2\n");
	    });

	rg.compile();

	context.m_command_pool.reset();
	context.m_command_buffer.begin();
	{
		rg.execute(context.m_command_buffer);
	}
	context.m_command_buffer.end();

	printf("Sandbox completed\n");
}
