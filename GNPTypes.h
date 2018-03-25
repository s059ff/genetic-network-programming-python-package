#pragma once

#include <cstdint>
#include <random>

#include <Eigen/Core>

namespace gnp
{
typedef float float32_t;
typedef double float64_t;

#ifdef GNP_USE_DOUBLE_PRECISION
typedef float32_t numeric_t;
typedef int32_t category_t;
#else
typedef float64_t numeric_t;
typedef int64_t category_t;
#endif

static_assert(sizeof(category_t) == sizeof(numeric_t), "");

union data_t {
    numeric_t numeric;
    category_t category;
};

inline bool operator==(data_t a, data_t b)
{
#ifdef GNP_USE_DOUBLE_PRECISION
    return reinterpret_cast<int64_t&>(a) == reinterpret_cast<int64_t&>(b);
#else
    return reinterpret_cast<int32_t&>(a) == reinterpret_cast<int32_t&>(b);
#endif    
}

inline bool operator!=(data_t a, data_t b)
{
#ifdef GNP_USE_DOUBLE_PRECISION
    return reinterpret_cast<int64_t&>(a) != reinterpret_cast<int64_t&>(b);
#else
    return reinterpret_cast<int32_t&>(a) != reinterpret_cast<int32_t&>(b);
#endif    
}

#if defined(__x86_64__) || defined(_WIN64)
typedef std::mt19937_64 randomizer_t;
#else
typedef std::mt19937 randomizer_t;
#endif

template <typename T>
using Matrix = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

template <typename T>
using Vector = Eigen::Matrix<T, 1, Eigen::Dynamic, Eigen::RowMajor>;
}
