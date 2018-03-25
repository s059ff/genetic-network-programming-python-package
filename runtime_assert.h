#pragma once

#include <cstdio>
#include <string>

#define runtime_assert(...) custom_runtime_assert(__FILE__, __LINE__, __VA_ARGS__)

inline void custom_runtime_assert(const char *__file__, int __line__, bool expression, const std::string &message = "Runtime assertion failed.")
{
    if (!expression)
    {
        fprintf(stderr, "%s (%s, %d)\n", message.c_str(), __file__, __line__);
        fflush(stderr);
        exit(1);
    }
}
