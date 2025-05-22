#include <cassert>
#include <cassert>
#include <random>
#include <stdarg.h>

#include "util.h"

void assert_if(bool st, const char *e, ...)
{
	if (!st)
	{
		return;
	}

	printf("[ERROR]: ");
	va_list arglist;
	{
		va_start(arglist, e);
		vprintf(e, arglist);
		va_end(arglist);
	}
	printf("\n");

	assert(false);
}

float random_float()
{
	static std::random_device rd = {};
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	return dist(gen);
}
