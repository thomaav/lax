#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define UNUSED(x) (void)x

static inline void terminate(const char *e, ...)
{
	printf("[ERROR]: ");
	va_list arglist;
	{
		va_start(arglist, e);
		vprintf(e, arglist);
		va_end(arglist);
	}
	printf("\n");

	exit(1);
}

static inline void info(const char *e, ...)
{
	printf("[INFO]: ");
	va_list arglist;
	{
		va_start(arglist, e);
		vprintf(e, arglist);
		va_end(arglist);
	}
	printf("\n");
}
