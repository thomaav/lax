#include "editor.h"

int main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	editor editor = {};
	editor.build();

	while (editor.m_context.m_window.step())
	{
		editor.update();
		editor.draw();
	}
}
