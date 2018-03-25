#pragma once

#include <string>
#include <vector>

#include "DataAttribute.h"

namespace gnp
{
// 属性情報の集合。
class DataAttributeCollection : public std::vector<DataAttribute>
{
    using base = std::vector<DataAttribute>;

  public:
    std::string to_string() const;
};
}
