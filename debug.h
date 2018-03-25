#pragma once

#include <iostream>

#define TRACE() _LOG(__FILE__, __LINE__)

#define LOG(...) _LOG(__FILE__, __LINE__, __VA_ARGS__)

inline void _LOG(const char *__file__, int __line__)
{
    std::cerr << __file__ << '(' << __line__ << ')' << std::endl;
}

template <typename T, typename... Args>
inline void _LOG(const char *__file__, int __line__, const T &a, const Args &... args)
{
    std::cerr << a << ' ';
    _LOG(__file__, __line__, args...);
}
