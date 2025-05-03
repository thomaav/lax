#if defined(__APPLE__)
#include <renderer/metal/context.h>
#elif defined(__linux__)
#include <renderer/vulkan/context.h>
#endif
#include <utils/util.h>

int main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

#if defined(__APPLE__)
	metal::context ctx = {};
	ctx.build();
	ctx.backend_test();
#elif defined(__linux__)
	vulkan::context ctx = {};
	ctx.build();
	ctx.backend_test();
#else
	printf("Running backend test on unrecognized platform, exiting\n");
	exit(1);
#endif
}
