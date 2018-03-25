#include <sstream>

#include "Genome.h"
#include "NodeGene.h"
#include "assert.h"
#include "format.h"
#include "runtime_assert.h"

namespace gnp
{
static inline double dice(randomizer_t &randomizer)
{
    return std::uniform_real_distribution<double>(0.0, 1.0)(randomizer);
}

template <typename T>
T dice(randomizer_t &randomizer, T min, T max);

template <>
numeric_t dice(randomizer_t &randomizer, numeric_t min, numeric_t max)
{
    return std::uniform_real_distribution<numeric_t>(min, max)(randomizer);
}

template <>
category_t dice(randomizer_t &randomizer, category_t min, category_t max)
{
    return std::uniform_int_distribution<category_t>(min, max)(randomizer);
}

static inline int dice(randomizer_t &randomizer, int max)
{
    return std::uniform_int_distribution<int>(0, max - 1)(randomizer);
}

static inline int dice(randomizer_t &randomizer, const std::vector<double> &probabilities)
{
    assert(0 < std::accumulate(probabilities.begin(), probabilities.end(), 0.0), "All probabilities is 0.");
    return std::discrete_distribution<int>(probabilities.begin(), probabilities.end())(randomizer);
}

const AbstractNodeGene *InitialNodeGene::next(const Vector<data_t> &values) const
{
    assert(this->target < this->owner->genes.size(), "Index is out of range.");
    return this->owner->genes[this->target].get();
}

const AbstractNodeGene *ProcessingNodeGene::next(const Vector<data_t> &values) const
{
    assert(this->target < this->owner->genes.size(), "Index is out of range.");
    return this->owner->genes[this->target].get();
}

const AbstractNodeGene *CategoryJudgementNodeGene::next(const Vector<data_t> &values) const
{
    auto data = values[this->source];
    auto value = data.category;
    auto index = this->branches.at(value);
    assert(index < this->targets.size(), "Index is out of range.");
    assert(this->targets[index] < this->owner->genes.size(), "Index is out of range.");
    return this->owner->genes[this->targets[index]].get();
}

const AbstractNodeGene *NumericJudgementNodeGene::next(const Vector<data_t> &values) const
{
    auto data = values[this->source];
    auto value = data.numeric;
    auto index = 0;
    for (index = 0; index < this->thresholds.size(); index++)
    {
        if (value < this->thresholds[index])
            break;
    }
    assert(index < this->targets.size(), "Index is out of range.");
    return this->owner->genes[this->targets[index]].get();
}

void InitialNodeGene::mutate(randomizer_t &randomizer, const GNPConfig &config, bool force_mutation)
{
    // ランダムに接続先ノードを設定する。
    if (force_mutation || dice(randomizer) < config.branch_mutation_rate)
    {
        auto num_genes = this->owner->genes.size();
        auto probabilities = std::vector<double>(num_genes, 1.0);
        probabilities[0] = 0.0;           // Forbid connection to initial node.
        probabilities[this->index] = 0.0; // Forbid self loop.
        this->target = dice(randomizer, probabilities);
    }
}

void ProcessingNodeGene::mutate(randomizer_t &randomizer, const GNPConfig &config, bool force_mutation)
{
    // ランダムに接続先ノードを設定する。
    if (force_mutation || dice(randomizer) < config.branch_mutation_rate)
    {
        auto num_genes = this->owner->genes.size();
        auto probabilities = std::vector<double>(num_genes, 1.0);
        probabilities[0] = 0.0;           // Forbid connection to initial node.
        probabilities[this->index] = 0.0; // Forbid self loop.
        this->target = dice(randomizer, probabilities);
    }

    // ランダムに出力値を設定する。
    auto num_outputs = config.output_attributes.size();
    this->value.conservativeResize(num_outputs);
    for (int i = 0; i < num_outputs; i++)
    {
        if (force_mutation || dice(randomizer) < config.output_mutation_rate)
        {
            auto &attribute = config.output_attributes[i];
            switch (attribute.type)
            {
            case DataAttributeType::Category:
            {
                auto min = attribute.min.category;
                auto max = attribute.max.category;
                this->value[i].category = dice<category_t>(randomizer, min, max);
                break;
            }
            case DataAttributeType::Numeric:
            {
                auto min = attribute.min.numeric;
                auto max = attribute.max.numeric;
                this->value[i].numeric = dice<numeric_t>(randomizer, min, max);
                break;
            }
            default:
                assert(false);
                break;
            }
        }
    }
}

void AbstractJudgementNodeGene::mutate(randomizer_t &randomizer, const GNPConfig &config, bool force_mutation)
{
    // ランダムに接続先ノードを設定する。
    this->targets.resize(config.num_branches);
    for (auto &target : this->targets)
    {
        if (force_mutation || dice(randomizer) < config.branch_mutation_rate)
        {
            auto num_genes = this->owner->genes.size();
            auto probabilities = std::vector<double>(num_genes, 1.0);
            probabilities[0] = 0.0;           // Forbid connection to initial node.
            probabilities[this->index] = 0.0; // Forbid connection of self loop.
            target = dice(randomizer, probabilities);
        }
    }
}

void CategoryJudgementNodeGene::mutate(randomizer_t &randomizer, const GNPConfig &config, bool force_mutation)
{
    base::mutate(randomizer, config, force_mutation);

    // ランダムに参照する入力データのインデックスを設定する。
    bool reference_is_changed = false;
    if (force_mutation || dice(randomizer) < config.data_source_mutation_rate)
    {
        auto num_inputs = config.input_attributes.size();
        auto probabilities = std::vector<double>(num_inputs, 0.0);
        for (int i = 0; i < num_inputs; i++)
            if (config.input_attributes[i].type == DataAttributeType::Category)
                probabilities[i] = 1.0;
        this->source = dice(randomizer, probabilities);

        reference_is_changed = true;
    }

    // ランダムに分岐関数の内部パラメータを設定する。
    auto &attribute = config.input_attributes[this->source];
    auto min = attribute.min.category;
    auto max = attribute.max.category;
    if (force_mutation || reference_is_changed)
        this->branches.clear();
    for (auto category = min; category <= max; category++)
    {
        if (force_mutation || reference_is_changed || dice(randomizer) < config.judgement_function_mutation_rate)
        {
            auto index = dice(randomizer, config.num_branches);
            this->branches[category] = index;
        }
    }
}

void NumericJudgementNodeGene::mutate(randomizer_t &randomizer, const GNPConfig &config, bool force_mutation)
{
    base::mutate(randomizer, config, force_mutation);

    // ランダムに参照する入力データのインデックスを設定する。
    bool reference_is_changed = false;
    if (force_mutation || dice(randomizer) < config.data_source_mutation_rate)
    {
        auto num_inputs = config.input_attributes.size();
        auto probabilities = std::vector<double>(num_inputs, 0.0);
        for (int i = 0; i < num_inputs; i++)
            if (config.input_attributes[i].type == DataAttributeType::Numeric)
                probabilities[i] = 1.0;
        this->source = dice(randomizer, probabilities);

        reference_is_changed = true;
    }

    // ランダムに分岐関数の内部パラメータを設定する。
    if (force_mutation || reference_is_changed || dice(randomizer) < config.judgement_function_mutation_rate)
    {
        this->thresholds.resize(config.num_branches - 1); // 注; しきい値の個数は分岐数 - 1
        auto &attribute = config.input_attributes[this->source];
        auto min = attribute.min.numeric;
        auto max = attribute.max.numeric;
        for (int i = 0; i < this->thresholds.size(); i++)
        {
            auto threshold = dice<numeric_t>(randomizer, min, max);
            this->thresholds[i] = threshold;
        }
    }
    std::sort(this->thresholds.begin(), this->thresholds.end());
}

std::unique_ptr<AbstractNodeGene> InitialNodeGene::duplicate(const Genome *owner) const
{
    auto ptr = std::unique_ptr<AbstractNodeGene>(new InitialNodeGene(*this));
    ptr->owner = owner;
    return ptr;
}

std::unique_ptr<AbstractNodeGene> ProcessingNodeGene::duplicate(const Genome *owner) const
{
    auto ptr = std::unique_ptr<AbstractNodeGene>(new ProcessingNodeGene(*this));
    ptr->owner = owner;
    return ptr;
}

std::unique_ptr<AbstractNodeGene> CategoryJudgementNodeGene::duplicate(const Genome *owner) const
{
    auto ptr = std::unique_ptr<AbstractNodeGene>(new CategoryJudgementNodeGene(*this));
    ptr->owner = owner;
    return ptr;
}

std::unique_ptr<AbstractNodeGene> NumericJudgementNodeGene::duplicate(const Genome *owner) const
{
    auto ptr = std::unique_ptr<AbstractNodeGene>(new NumericJudgementNodeGene(*this));
    ptr->owner = owner;
    return ptr;
}

void AbstractNodeGene::serialize(picojson::object &object, const GNPConfig &config) const
{
    object["typeid"] = picojson::value(typeid(*this).name());
    object["index"] = picojson::value(static_cast<double>(this->index));
    object["delay"] = picojson::value(static_cast<double>(this->delay));
}

void AbstractNodeGene::deserialize(const picojson::object &object, const GNPConfig &config)
{
    runtime_assert(object.at("typeid").get<std::string>() == typeid(*this).name());
    this->owner = nullptr;
    this->index = static_cast<int>(object.at("index").get<double>());
    this->delay = static_cast<double>(object.at("delay").get<double>());
}

void InitialNodeGene::serialize(picojson::object &object, const GNPConfig &config) const
{
    base::serialize(object, config);

    object["target"] = picojson::value(static_cast<double>(this->target));
}

void InitialNodeGene::deserialize(const picojson::object &object, const GNPConfig &config)
{
    base::deserialize(object, config);

    this->target = static_cast<int>(object.at("target").get<double>());
}

void ProcessingNodeGene::serialize(picojson::object &object, const GNPConfig &config) const
{
    base::serialize(object, config);

    object["target"] = picojson::value(static_cast<double>(this->target));
    picojson::array value(this->value.size());
    for (int i = 0; i < this->value.size(); i++)
    {
        switch (config.output_attributes[i].type)
        {
        case DataAttributeType::Numeric:
            value[i] = picojson::value(static_cast<double>(this->value[i].numeric));
            break;
        case DataAttributeType::Category:
            value[i] = picojson::value(static_cast<double>(this->value[i].category));
            break;
        default:
            assert(false);
            break;
        }
    }
    object["value"] = picojson::value(value);
}

void ProcessingNodeGene::deserialize(const picojson::object &object, const GNPConfig &config)
{
    base::deserialize(object, config);

    this->target = static_cast<int>(object.at("target").get<double>());

    auto value = object.at("value").get<picojson::array>();
    this->value.resize(value.size());
    for (int i = 0; i < value.size(); i++)
    {
        switch (config.output_attributes[i].type)
        {
        case DataAttributeType::Numeric:
            this->value[i].numeric = static_cast<numeric_t>(value[i].get<double>());
            break;
        case DataAttributeType::Category:
            this->value[i].category = static_cast<category_t>(value[i].get<double>());
            break;
        default:
            assert(false);
            break;
        }
    }
}

void AbstractJudgementNodeGene::serialize(picojson::object &object, const GNPConfig &config) const
{
    base::serialize(object, config);

    picojson::array targets(this->targets.size());
    std::transform(this->targets.begin(), this->targets.end(), targets.begin(), [](auto target) {
        return picojson::value(static_cast<double>(target));
    });
    object["targets"] = picojson::value(targets);
    object["source"] = picojson::value(static_cast<double>(this->source));
}

void AbstractJudgementNodeGene::deserialize(const picojson::object &object, const GNPConfig &config)
{
    base::deserialize(object, config);

    auto &targets = object.at("targets").get<picojson::array>();
    this->targets.resize(targets.size());
    std::transform(targets.begin(), targets.end(), this->targets.begin(), [](auto target) {
        return static_cast<int>(target.template get<double>());
    });
    this->source = static_cast<int>(object.at("source").get<double>());
}

void CategoryJudgementNodeGene::serialize(picojson::object &object, const GNPConfig &config) const
{
    base::serialize(object, config);

    picojson::array branches(this->branches.size());
    std::transform(this->branches.begin(), this->branches.end(), branches.begin(), [](auto pair) {
        picojson::object obj;
        obj["first"] = picojson::value(static_cast<double>(pair.first));
        obj["second"] = picojson::value(static_cast<double>(pair.second));
        return picojson::value(obj);
    });
    object["branches"] = picojson::value(branches);
}

void CategoryJudgementNodeGene::deserialize(const picojson::object &object, const GNPConfig &config)
{
    base::deserialize(object, config);

    auto &branches = object.at("branches").get<picojson::array>();
    this->branches.clear();
    for (auto &obj : branches)
    {
        auto first = static_cast<category_t>(obj.get<picojson::object>().at("first").get<double>());
        auto second = static_cast<int>(obj.get<picojson::object>().at("second").get<double>());
        this->branches[first] = second;
    }
}

void NumericJudgementNodeGene::serialize(picojson::object &object, const GNPConfig &config) const
{
    base::serialize(object, config);

    picojson::array thresholds(this->thresholds.size());
    std::transform(this->thresholds.begin(), this->thresholds.end(), thresholds.begin(), [](auto threshold) {
        return picojson::value(static_cast<double>(threshold));
    });
    object["thresholds"] = picojson::value(thresholds);
}

void NumericJudgementNodeGene::deserialize(const picojson::object &object, const GNPConfig &config)
{
    base::deserialize(object, config);

    auto &thresholds = object.at("thresholds").get<picojson::array>();
    this->thresholds.resize(thresholds.size());
    std::transform(thresholds.begin(), thresholds.end(), this->thresholds.begin(), [](auto &threshold) {
        return static_cast<numeric_t>(threshold.template get<double>());
    });
}

bool AbstractNodeGene::equal_to(const AbstractNodeGene *other) const
{
    return this->index == other->index && this->delay == other->delay;
}

bool InitialNodeGene::equal_to(const AbstractNodeGene *_other) const
{
    auto other = dynamic_cast<decltype(this)>(_other);
    return other != nullptr && base::equal_to(other) && this->target == other->target;
}

bool ProcessingNodeGene::equal_to(const AbstractNodeGene *_other) const
{
    auto other = dynamic_cast<decltype(this)>(_other);
    return other != nullptr && base::equal_to(other) && this->target == other->target && this->value == other->value;
}

bool AbstractJudgementNodeGene::equal_to(const AbstractNodeGene *_other) const
{
    auto other = dynamic_cast<decltype(this)>(_other);
    return other != nullptr && base::equal_to(other) && this->targets == other->targets && this->source == other->source;
}

bool CategoryJudgementNodeGene::equal_to(const AbstractNodeGene *_other) const
{
    auto other = dynamic_cast<decltype(this)>(_other);
    return other != nullptr && base::equal_to(other) && this->branches == other->branches;
}

bool NumericJudgementNodeGene::equal_to(const AbstractNodeGene *_other) const
{
    auto other = dynamic_cast<decltype(this)>(_other);
    return other != nullptr && base::equal_to(other) && this->thresholds == other->thresholds;
}

bool AbstractNodeGene::not_equal_to(const AbstractNodeGene *other) const
{
    return !this->equal_to(other);
}
}
