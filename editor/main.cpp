#include <renderer/sandbox.h>

#include "editor.h"

int main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	run_sandbox();
	exit(0);

	editor editor = {};
	editor.build();

	while (editor.m_context.m_window.step())
	{
		editor.update();
		editor.draw();
	}
}
