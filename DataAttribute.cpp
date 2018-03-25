#include <sstream>

#include "DataAttribute.h"
#include "format.h"
#include "runtime_assert.h"

namespace gnp
{
static std::string to_string(DataAttributeType type)
{
    switch (type)
    {
    case DataAttributeType::Numeric:
        return "numeric";
    case DataAttributeType::Category:
        return "category";
    default:
        assert(false);
        return "unknown";
    }
}

static DataAttributeType to_type(std::string _typename)
{
    std::transform(_typename.begin(), _typename.end(), _typename.begin(), ::tolower);
    if (_typename == "numeric")
        return DataAttributeType::Numeric;
    if (_typename == "category")
        return DataAttributeType::Category;
    assert(false);
    return DataAttributeType::Unknown;
}

DataAttribute::DataAttribute(
    const std::string &name,
    const std::string &_typename,
    const double min,
    const double max,
    const std::vector<std::string> &labels)
    : name(name), type(to_type(_typename)), min({0}), max({0}), labels(labels)
{
    switch (this->type)
    {
    case DataAttributeType::Category:
        if (labels.empty())
        {
            const_cast<data_t &>(this->min).category = (category_t)min;
            const_cast<data_t &>(this->max).category = (category_t)max;
            runtime_assert(this->min.category == 0, "Minimum of category must be 0.");
        }
        else
        {
            const_cast<data_t &>(this->min).category = (category_t)0;
            const_cast<data_t &>(this->max).category = (category_t)(labels.size() - 1);
        }
        break;
    case DataAttributeType::Numeric:
        const_cast<data_t &>(this->min).numeric = (numeric_t)min;
        const_cast<data_t &>(this->max).numeric = (numeric_t)max;
        break;
    default:
        runtime_assert(false, format("'{0}' is invalid typename.", _typename));
        break;
    }
}

std::string DataAttribute::get_name() const
{
    return this->name;
}

std::string DataAttribute::get_typename() const
{
    return gnp::to_string(this->type);
}

double DataAttribute::get_min() const
{
    switch (this->type)
    {
    case DataAttributeType::Numeric:
        return (double)this->min.numeric;
    case DataAttributeType::Category:
        return (double)this->min.category;
    default:
        return (double)0.0;
    }
}

double DataAttribute::get_max() const
{
    switch (this->type)
    {
    case DataAttributeType::Numeric:
        return (double)this->max.numeric;
    case DataAttributeType::Category:
        return (double)this->max.category;
    default:
        assert(false);
        return (double)0.0;
    }
}

boost::python::list DataAttribute::get_labels() const
{
    namespace py = boost::python;
    py::list list_py;
    for (auto &label : this->labels)
    {
        list_py.append(py::str(label));
    }
    return list_py;
}

std::string DataAttribute::to_string() const
{
    auto join = [](auto &container) {
        std::stringstream stream;
        stream << "[";
        for (auto it = container.begin(); it != container.end(); it++)
        {
            stream << *it;
            if (it != std::prev(container.end()))
                stream << ", ";
        }
        stream << "]";
        return stream.str();
    };

    std::stringstream stream;
    stream << "name: " << this->name << std::endl;
    stream << "type: " << this->get_typename() << std::endl;
    stream << "min: " << this->get_min() << std::endl;
    stream << "max: " << this->get_max() << std::endl;
    stream << "labels: " << join(this->labels) << std::endl;
    return stream.str();
}
}
