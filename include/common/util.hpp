#pragma once

#include <stdio.h>
#include <cstdlib>

#if defined(__GNUC__) || defined(__clang__)
#define UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#define UNREACHABLE() __assume(false)
#else
#define UNREACHABLE() std::abort()
#endif

[[noreturn]] void unreachable(const char* message = "[BUG] unreachable");

#define ASSERT(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "Assertion failed: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
            std::abort(); \
        } \
    } while (0)