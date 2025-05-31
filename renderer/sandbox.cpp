#include "sandbox.h"
#include "render_graph.h"

void run_sandbox()
{
	render_graph rg = {};
	render_pass &rp1 = rg.add_render_pass("main");
	rp1.add_buffer("constants1");
	rp1.add_texture("color1");
	render_pass &rp2 = rg.add_render_pass("post");
	rp1.add_buffer("constants2");
	rp1.add_texture("color1");
	rp1.add_texture("color2");
}
