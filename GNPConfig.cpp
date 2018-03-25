#include <fstream>
#include <sstream>
#include <string>

#include <picojson.h>

#include "GNPConfig.h"
#include "format.h"
#include "runtime_assert.h"

namespace gnp
{
template <typename Type, typename Map>
Type extract_numeric(const Map &map, const std::string &key)
{
    auto it = map.find(key);
    runtime_assert(it != map.end(), format("Key({0}) is not found.", key));
    return static_cast<Type>(it->second.template get<double>());
}

template <typename Type, typename Map>
std::vector<Type> extract_numerics(const Map &map, const std::string &key)
{
    auto it = map.find(key);
    runtime_assert(it != map.end(), "Key is not found.");
    auto ary = it->second.template get<picojson::array>();
    auto vec = std::vector<Type>(ary.size());
    for (int i = 0; i < ary.size(); i++)
        vec[i] = static_cast<Type>(ary[i].template get<double>());
    return vec;
}

template <typename Map>
std::vector<std::string> extract_strings(const Map &map, const std::string &key)
{
    auto it = map.find(key);
    runtime_assert(it != map.end(), "Key is not found.");
    auto ary = it->second.template get<picojson::array>();
    auto vec = std::vector<std::string>(ary.size());
    for (int i = 0; i < ary.size(); i++)
        vec[i] = ary[i].template get<std::string>();
    return vec;
}

template <typename Map>
bool exists(const Map &map, const std::string &key)
{
    auto it = map.find(key);
    return it != map.end();
}

class no_limitation_t
{
} no_limitation;

template <typename T>
bool range_validation(T value, T min, T max)
{
    return (min <= value) && (value <= max);
}

template <typename T>
bool range_validation(T value, no_limitation_t, T max)
{
    return (value <= max);
}

template <typename T>
bool range_validation(T value, T min, no_limitation_t)
{
    return (min <= value);
}

GNPConfig::GNPConfig(const char *path)
{
    picojson::value json;
    std::ifstream stream(path, std::ios::binary);
    stream >> json;
    stream.close();

    auto root = json.get<picojson::object>();
    for (const auto &value : root.at("input_attributes").get<picojson::array>())
    {
        auto &obj = value.get<picojson::object>();
        auto name = obj.at("name").get<std::string>();
        auto typename_ = obj.at("typename").get<std::string>();
        auto min = exists(obj, "min") ? obj.at("min").get<double>() : 0.0;
        auto max = exists(obj, "max") ? obj.at("max").get<double>() : 0.0;
        auto labels = exists(obj, "labels") ? extract_strings(obj, "labels") : std::vector<std::string>();
        this->input_attributes.emplace_back(name, typename_, min, max, labels);
    }
    for (const auto &value : root.at("output_attributes").get<picojson::array>())
    {
        auto &obj = value.get<picojson::object>();
        auto name = obj.at("name").get<std::string>();
        auto typename_ = obj.at("typename").get<std::string>();
        auto min = exists(obj, "min") ? obj.at("min").get<double>() : 0.0;
        auto max = exists(obj, "max") ? obj.at("max").get<double>() : 0.0;
        auto labels = exists(obj, "labels") ? extract_strings(obj, "labels") : std::vector<std::string>();
        this->output_attributes.emplace_back(name, typename_, min, max, labels);
    }
    this->num_genomes = extract_numeric<int>(root, "num_genomes");
    this->num_elites = extract_numeric<int>(root, "num_elites");
    this->num_category_judgement_nodes = extract_numeric<int>(root, "num_category_judgement_nodes");
    this->num_numeric_judgement_nodes = extract_numeric<int>(root, "num_numeric_judgement_nodes");
    this->num_processing_nodes = extract_numeric<int>(root, "num_processing_nodes");
    this->num_branches = extract_numeric<int>(root, "num_branches");
    this->crossover_rate = extract_numeric<double>(root, "crossover_rate");
    this->branch_mutation_rate = extract_numeric<double>(root, "branch_mutation_rate");
    this->data_source_mutation_rate = extract_numeric<double>(root, "data_source_mutation_rate");
    this->judgement_function_mutation_rate = extract_numeric<double>(root, "judgement_function_mutation_rate");
    this->output_mutation_rate = extract_numeric<double>(root, "output_mutation_rate");
    this->time_limit = extract_numeric<double>(root, "time_limit");
    this->delay_time_processing_node = extract_numeric<double>(root, "delay_time_processing_node");
    this->delay_time_judgement_node = extract_numeric<double>(root, "delay_time_judgement_node");

    // 設定に矛盾や無効な値がないか検証します。
    for (auto &attr : this->input_attributes)
    {
        runtime_assert(attr.type != DataAttributeType::Category || attr.min.category == 0);
        runtime_assert(attr.type != DataAttributeType::Category || range_validation<category_t>(attr.max.category, 0, no_limitation));
        runtime_assert(attr.type != DataAttributeType::Category || attr.min.category <= attr.max.category);
        runtime_assert(attr.type != DataAttributeType::Numeric || attr.min.numeric <= attr.max.numeric);
    }
    for (auto &attr : this->output_attributes)
    {
        runtime_assert(attr.type != DataAttributeType::Category || attr.min.category == 0);
        runtime_assert(attr.type != DataAttributeType::Category || range_validation<category_t>(attr.max.category, 0, no_limitation));
        runtime_assert(attr.type != DataAttributeType::Category || attr.min.category <= attr.max.category);
        runtime_assert(attr.type != DataAttributeType::Numeric || attr.min.numeric <= attr.max.numeric);
    }
    runtime_assert(range_validation<int>(this->num_genomes, 1, no_limitation));
    runtime_assert(range_validation<int>(this->num_elites, 0, no_limitation));
    runtime_assert(range_validation<int>(this->num_category_judgement_nodes, 0, no_limitation));
    runtime_assert(range_validation<int>(this->num_numeric_judgement_nodes, 0, no_limitation));
    runtime_assert(range_validation<int>(this->num_processing_nodes, 1, no_limitation));
    runtime_assert(range_validation<int>(this->num_branches, 1, no_limitation));
    runtime_assert(range_validation<double>(this->crossover_rate, 0, 1));
    runtime_assert(range_validation<double>(this->branch_mutation_rate, 0, 1));
    runtime_assert(range_validation<double>(this->data_source_mutation_rate, 0, 1));
    runtime_assert(range_validation<double>(this->judgement_function_mutation_rate, 0, 1));
    runtime_assert(range_validation<double>(this->output_mutation_rate, 0, 1));
    runtime_assert(range_validation<double>(this->time_limit, 0, no_limitation));
    runtime_assert(range_validation<double>(this->delay_time_processing_node, 0, no_limitation));
    runtime_assert(range_validation<double>(this->delay_time_judgement_node, 0, no_limitation));
}

std::string GNPConfig::to_string() const
{
    std::stringstream stream;
    stream << "input attributes: " << std::endl;
    stream << this->input_attributes.to_string();
    stream << "output attributes: " << std::endl;
    stream << this->output_attributes.to_string();
    stream << "num_genomes: " << this->num_genomes << std::endl;
    stream << "num_elites: " << this->num_elites << std::endl;
    stream << "num_category_judgement_nodes: " << this->num_category_judgement_nodes << std::endl;
    stream << "num_numeric_judgement_nodes: " << this->num_numeric_judgement_nodes << std::endl;
    stream << "num_processing_nodes: " << this->num_processing_nodes << std::endl;
    stream << "num_branches: " << this->num_branches << std::endl;
    stream << "crossover_rate: " << this->crossover_rate << std::endl;
    stream << "branch_mutation_rate: " << this->branch_mutation_rate << std::endl;
    stream << "data_source_mutation_rate: " << this->data_source_mutation_rate << std::endl;
    stream << "judgement_function_mutation_rate: " << this->judgement_function_mutation_rate << std::endl;
    stream << "output_mutation_rate: " << this->output_mutation_rate << std::endl;
    stream << "time_limit: " << this->time_limit << std::endl;
    stream << "delay_time_processing_node: " << this->delay_time_processing_node << std::endl;
    stream << "delay_time_judgement_node: " << this->delay_time_judgement_node << std::endl;

    return stream.str();
}
}
