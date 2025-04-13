#include <renderer/vulkan/context.h>
#include <utils/util.h>

int main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	vulkan::context ctx = {};
	ctx.build();
	ctx.backend_test();
}
