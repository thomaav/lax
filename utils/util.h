#pragma once

#include <cassert>
#include <memory>
#include <random>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define UNUSED(x) (void)x

static inline void assert_if(bool st, const char *e, ...)
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

template <typename T> using uref = std::unique_ptr<T>;
template <typename T, typename... Args> std::unique_ptr<T> make_uref(Args &&...args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}
template <typename T> using ref = std::shared_ptr<T>;
template <typename T, typename... Args> std::shared_ptr<T> make_ref(Args &&...args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}
