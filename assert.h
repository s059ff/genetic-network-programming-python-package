#pragma once

#undef assert

#include <cstdio>
#include <string>

#define assert(...) custom_assert(__FILE__, __LINE__, __VA_ARGS__)

#ifndef NDEBUG

inline void custom_assert(const char *__file__, int __line__, bool expression, const std::string &message = "Assertion failed.")
{
    if (!expression)
    {
        fprintf(stderr, "%s (%s, %d)\n", message.c_str(), __file__, __line__);
        fflush(stderr);
        exit(1);
    }
}

#else

template <typename... Args>
inline void custom_assert(const Args &...)
{
    // Do nothing.
}

#endif
