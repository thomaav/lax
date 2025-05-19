#pragma once

#include <cassert>
#include <random>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define UNUSED(x) (void)x

void assert_if(bool st, const char *e, ...);
float random_float();
