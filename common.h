// Common utility typedefs and macros and functions and includes
#ifndef COMMON_H
#define COMMON_H

#include <cstdlib>
#include <cstdio>

extern "C" {
#include <simple.h>
#include "print.h"
}

// toggle logging macro
#if 0
#define LOG(...) do {fprintf(stdout, __VA_ARGS__);} while(0)
#else
#define LOG(...) do {} while(0)
#endif

// toggle custom asserts
#if 1
#define ASSERT(c) do {if (!(c)) {fprintf(stderr, "%s:%d: ASSERT failed: \"%s\"\n", __FILE__, __LINE__, #c); exit(1);}} while(0)
#else
#define ASSERT(c) do {} while(0)
#endif

// convenience for all those for loops over containers
#define FOREACH(type, var, container) for(type var = container.begin(); var != container.end(); ++var)

typedef unsigned int uint;
typedef unsigned long ulong;

#endif
