#pragma once

#include <random>
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

/* (TODO, thoave01): util.cpp. */
static inline float random_float()
{
	static std::random_device rd = {};
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	return dist(gen);
}
