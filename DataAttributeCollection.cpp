#include <sstream>

#include "DataAttributeCollection.h"

namespace gnp
{
std::string DataAttributeCollection::to_string() const
{
    std::stringstream stream;
    for (size_t i = 0; i < this->size(); i++)
    {
        stream << '[' << i << ']' << std::endl;
        stream << this->at(i).to_string();
    }
    return stream.str();
}
}
