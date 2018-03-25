#pragma once

#include <string>
#include <utility>
#include <vector>

#include <boost/python.hpp>

#include "GNPTypes.h"

namespace gnp
{
// 属性の種類。
enum class DataAttributeType
{
    Unknown,  // (Invalid data.)
    Category, // カテゴリ属性。
    Numeric   // 数値属性。
};

// 属性情報。
class DataAttribute
{
  public:
    DataAttribute() = default;

    DataAttribute(
        const std::string &name,
        const std::string &_typename,
        const double min,
        const double max,
        const std::vector<std::string> &labels);

  public:
    std::string get_name() const;

    std::string get_typename() const;

    double get_min() const;

    double get_max() const;

    boost::python::list get_labels() const;

    std::string to_string() const;

  public:
    // 属性名。
    const std::string name;

    // 属性の種類。
    const DataAttributeType type = DataAttributeType::Unknown;

    // 最小値。
    const data_t min = {0};

    // 最大値。
    const data_t max = {0};

    // ラベルインデックスとラベルの対応付け(カテゴリ属性の場合のみ有効、ラベルが付与されていない場合は無効)。
    const std::vector<std::string> labels;
};
}
