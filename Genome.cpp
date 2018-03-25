#include <algorithm>
#include <fstream>
#include <iomanip>
#include <list>
#include <sstream>

#include <picojson.h>

#include "Genome.h"
#include "assert.h"
#include "format.h"
#include "runtime_assert.h"

namespace gnp
{
static inline double dice(randomizer_t &randomizer)
{
    return std::uniform_real_distribution<double>(0.0, 1.0)(randomizer);
}

void Genome::configure_new(randomizer_t &randomizer, const GNPConfig &config)
{
    this->allocate_memory(config);

    this->fitness = 0.0;
    std::for_each(this->genes.begin(), this->genes.end(), [&randomizer, &config](auto &gene) {
        constexpr bool force_mutation = true;
        gene->mutate(randomizer, config, force_mutation);
    });
}

void Genome::configure_inheritance(const Genome &parent)
{
    this->fitness = parent.fitness;

    auto num_genes = parent.genes.size();
    this->genes.clear();
    this->genes.reserve(num_genes);
    for (int i = 0; i < num_genes; i++)
    {
        this->genes.push_back(parent.genes[i]->duplicate(this));
    }
}

void Genome::configure_inheritance_move(Genome &&parent)
{
    this->fitness = std::move(parent.fitness);
    this->genes = std::move(parent.genes);
    for (auto &gene : this->genes)
        gene->owner = this;
    for (auto &gene : parent.genes)
        gene->owner = nullptr;
}

void Genome::configure_crossover(randomizer_t &randomizer, const Genome &parent1, const Genome &parent2)
{
    auto num_genes = parent1.genes.size();
    assert(num_genes == parent1.genes.size());
    assert(num_genes == parent2.genes.size());

    this->genes.clear();
    this->genes.reserve(num_genes);
    for (int i = 0; i < num_genes; i++)
    {
        if (dice(randomizer) < 0.5)
            this->genes.push_back(parent1.genes[i]->duplicate(this));
        else
            this->genes.push_back(parent2.genes[i]->duplicate(this));
    }
}

void Genome::mutate(randomizer_t &randomizer, const GNPConfig &config)
{
    for (auto &gene : this->genes)
        gene->mutate(randomizer, config);
}

void Genome::serialize(const char *path, const GNPConfig &config) const
{
    picojson::object object;
    this->serialize_to_object(object, config);
    std::ofstream stream(path);
    stream << picojson::value(object) << std::endl;
}

void Genome::serialize_to_object(picojson::object &object, const GNPConfig &config) const
{
    object["fitness"] = picojson::value(this->fitness);

    picojson::array genes(this->genes.size());
    std::transform(this->genes.begin(), this->genes.end(), genes.begin(), [&config](auto &gene) {
        picojson::object object;
        gene->serialize(object, config);
        return picojson::value(object);
    });
    object["genes"] = picojson::value(genes);
}

void Genome::deserialize(const char *path, const GNPConfig &config)
{
    std::ifstream stream(path);
    picojson::value value;
    stream >> value;
    this->deserialize_from_object(value.get<picojson::object>(), config);
}

void Genome::deserialize_from_object(const picojson::object &object, const GNPConfig &config)
{
    this->allocate_memory(config);
    this->fitness = object.at("fitness").get<double>();
    auto &genes = object.at("genes").get<picojson::array>();
    runtime_assert(this->genes.size() == genes.size());
    for (int i = 0; i < genes.size(); i++)
    {
        this->genes[i]->deserialize(genes[i].get<picojson::object>(), config);
    }
}

template <typename T, typename Container>
std::vector<const T *> filter(const Container &container)
{
    std::vector<const T *> results;
    results.reserve(container.size());
    for (auto &pointer : container)
    {
        auto ptr = dynamic_cast<const T *>(pointer.get());
        if (ptr)
            results.push_back(ptr);
    }
    return results;
}

void Genome::savefig(const char *path, const GNPConfig &config) const
{
    std::ofstream stream(path);
    stream << std::setprecision(2) << std::fixed;
    stream << "digraph G" << std::endl;
    stream << '{' << std::endl;
    stream << '\t' << "graph [];" << std::endl;
    stream << '\t' << "node [shape=circle, style=filled, fixedsize=true];" << std::endl;
    stream << '\t' << "edge [penwidth=1, style=solid];" << std::endl;
    for (auto &gene : filter<InitialNodeGene>(this->genes))
    {
        auto name = gene->index;
        stream << '\t' << name;
        stream << '[';
        stream << "shape=doublecircle, fillcolor=lightpink";
        stream << ']';
        stream << ';' << std::endl;

        auto source = gene->index;
        auto target = gene->target;
        stream << '\t' << source << "->" << target << ';' << std::endl;
    }
    for (auto &gene : filter<ProcessingNodeGene>(this->genes))
    {
        auto name = gene->index;
        stream << '\t' << name;
        stream << '[';
        stream << "shape=doublecircle, fillcolor=lightblue";
        stream << ']';
        stream << ';' << std::endl;

        auto source = gene->index;
        auto target = gene->target;
        stream << '\t' << source << "->" << target << ';' << std::endl;
    }
    for (auto &gene : filter<CategoryJudgementNodeGene>(this->genes))
    {
        auto name = gene->index;
        stream << '\t' << name;
        stream << '[';
        stream << "fillcolor=lightyellow";
        stream << ']';
        stream << ';' << std::endl;

        auto source = gene->index;
        for (auto pair : gene->branches)
        {
            auto target = gene->targets[pair.second];
            auto &attribute = config.input_attributes[gene->source];
            auto &name = attribute.name;
            auto &category = attribute.labels[pair.first];
            auto label = format("{0} is {1}", name, category);
            stream << '\t' << source << "->" << target;
            stream << '[';
            stream << "label=" << '"' << label << '"';
            stream << ']';
            stream << ';' << std::endl;
        }
    }
    for (auto &gene : filter<NumericJudgementNodeGene>(this->genes))
    {
        auto name = gene->index;
        stream << '\t' << name;
        stream << '[';
        stream << "fillcolor=lightcyan";
        stream << ']';
        stream << ';' << std::endl;
        
        auto source = gene->index;
        auto ftos = [](auto x) {
            std::stringstream stream;
            stream << std::setprecision(2) << std::fixed << x;
            return stream.str();
        };
        for (int i = 0; i < gene->targets.size(); i++)
        {
            auto target = gene->targets[i];
            auto &attribute = config.input_attributes[gene->source];
            auto &name = attribute.name;
            if (i == 0)
            {
                auto upper = gene->thresholds.front();
                auto label = format("{0} < {1}", name, ftos(upper));
                stream << '\t' << source << "->" << target;
                stream << '[';
                stream << "label=" << '"' << label << '"';
                stream << ']';
                stream << ';' << std::endl;
            }
            else if (i < gene->thresholds.size())
            {
                auto lower = gene->thresholds[i - 1];
                auto upper = gene->thresholds[i];
                auto label = format("{0} < {1} < {2}", ftos(lower), name, ftos(upper));
                stream << '\t' << source << "->" << target;
                stream << '[';
                stream << "label=" << '"' << label << '"';
                stream << ']';
                stream << ';' << std::endl;
            }
            else
            {
                auto lower = gene->thresholds.back();
                auto label = format("{0} < {1}", ftos(lower), name);
                stream << '\t' << source << "->" << target;
                stream << '[';
                stream << "label=" << '"' << label << '"';
                stream << ']';
                stream << ';' << std::endl;
            }
        }
    }
    stream << '}' << std::endl;
}

Matrix<data_t> Genome::activate(const Vector<data_t> &vector, const GNPConfig &config) const
{
    auto remaining_time = config.time_limit;
    const auto *current_node = this->genes.front().get();
    auto outputs = std::list<data_t>();

    while (0 < remaining_time)
    {
        assert(current_node->owner == this);
        if (typeid(*current_node) == typeid(ProcessingNodeGene))
        {
            auto &output = static_cast<const ProcessingNodeGene *>(current_node)->value;
            outputs.insert(outputs.end(), output.data(), output.data() + output.size());
        }
        remaining_time -= current_node->delay;
        current_node = current_node->next(vector);
    }

    auto cols = config.output_attributes.size();
    auto rows = outputs.size() / cols;
    assert(rows * cols == outputs.size());
    Matrix<data_t> _outputs(rows, cols);
    std::copy(outputs.begin(), outputs.end(), _outputs.data());
    return _outputs;
}

static Vector<data_t> pyvec2cppvec(const DataAttributeCollection &attributes, boost::python::numpy::ndarray vector_py)
{
    namespace py = boost::python;
    namespace np = boost::python::numpy;

    auto ndim = vector_py.get_nd();
    runtime_assert(ndim == 1, "ndim must be 1.");

    auto dims = py::len(vector_py);
    runtime_assert(attributes.size() == dims, "vector_py dimension do not match the length of attributes.");

    Vector<data_t> vector(dims);

    std::vector<DataAttributeType> types(dims);
    std::transform(attributes.begin(), attributes.end(), types.begin(), [](auto &attr) { return attr.type; });

    auto copy = [&attributes](const auto *source, data_t *dest) {
        int dims = attributes.size();
        for (int index = 0; index < dims; index++)
        {
            auto type = attributes[index].type;
            switch (type)
            {
            case DataAttributeType::Category:
                dest[index].category = (category_t)source[index];
                break;
            case DataAttributeType::Numeric:
                dest[index].numeric = (numeric_t)source[index];
                break;
            default:
                runtime_assert(false);
                break;
            }
        }
    };

    auto translate = [&attributes](boost::python::numpy::ndarray source, data_t *dest) {
        int dims = attributes.size();
        for (int index = 0; index < dims; index++)
        {
            auto type = attributes[index].type;
            switch (type)
            {
            case DataAttributeType::Category:
                dest[index].category = (category_t)py::extract<category_t>(source[index]);
                break;
            case DataAttributeType::Numeric:
                dest[index].numeric = (numeric_t)py::extract<numeric_t>(source[index]);
                break;
            default:
                runtime_assert(false);
                break;
            }
        }
    };

    auto dtype = vector_py.get_dtype();
    if (dtype == np::dtype::get_builtin<int8_t>())
    {
        auto source = reinterpret_cast<const int8_t *>(vector_py.get_data());
        copy(source, vector.data());
    }
    else if (dtype == np::dtype::get_builtin<int16_t>())
    {
        auto source = reinterpret_cast<const int16_t *>(vector_py.get_data());
        copy(source, vector.data());
    }
    else if (dtype == np::dtype::get_builtin<int32_t>())
    {
        auto source = reinterpret_cast<const int32_t *>(vector_py.get_data());
        copy(source, vector.data());
    }
    else if (dtype == np::dtype::get_builtin<int64_t>())
    {
        auto source = reinterpret_cast<const int64_t *>(vector_py.get_data());
        copy(source, vector.data());
    }
    else if (dtype == np::dtype::get_builtin<float32_t>())
    {
        auto source = reinterpret_cast<const float32_t *>(vector_py.get_data());
        copy(source, vector.data());
    }
    else if (dtype == np::dtype::get_builtin<float64_t>())
    {
        auto source = reinterpret_cast<const float64_t *>(vector_py.get_data());
        copy(source, vector.data());
    }
    else
    {
        // runtime_assert(false, "dtype must be int32, int64, float32, or float64.");
        translate(vector_py, vector.data());
    }

    return vector;
}

static boost::python::numpy::ndarray cppmat2pymat(const DataAttributeCollection &attributes, const Matrix<data_t> &mat)
{
    namespace py = boost::python;
    namespace np = boost::python::numpy;

    auto rows = mat.rows();
    auto cols = mat.cols();
    auto shape = py::make_tuple(rows, cols);
    auto dtype = np::dtype::get_builtin<double>();
    auto mat_py = np::zeros(shape, dtype);
    auto mat_py_ = Eigen::Map<Matrix<double>>(reinterpret_cast<double *>(mat_py.get_data()), rows, cols);

    for (int i = 0; i < mat.rows(); i++)
    {
        for (int j = 0; j < mat.cols(); j++)
        {
            auto type = attributes[j].type;
            switch (type)
            {
            case DataAttributeType::Category:
                mat_py_(i, j) = (double)mat(i, j).category;
                break;
            case DataAttributeType::Numeric:
                mat_py_(i, j) = (double)mat(i, j).numeric;
                break;
            default:
                runtime_assert(false);
                break;
            }
        }
    }

    return mat_py;
}

boost::python::numpy::ndarray Genome::activate_py(boost::python::numpy::ndarray vector_py, const GNPConfig &config) const
{
    auto input = pyvec2cppvec(config.input_attributes, vector_py);
    auto output = cppmat2pymat(config.output_attributes, this->activate(input, config));
    return output;
}

bool Genome::equal_to(const Genome &other) const
{
    auto &group1 = this->genes;
    auto &group2 = other.genes;
    if (group1.size() == group2.size())
    {
        std::vector<int> indices(group1.size());
        std::iota(indices.begin(), indices.end(), 0);
        return std::all_of(indices.begin(), indices.end(), [&group1, &group2](int index) {
            auto &instance1 = group1[index];
            auto &instance2 = group2[index];
            return instance1->equal_to(instance2.get());
        });
    }
    return false;
}

bool Genome::not_equal_to(const Genome &other) const
{
    auto &group1 = this->genes;
    auto &group2 = other.genes;
    if (group1.size() == group2.size())
    {
        std::vector<int> indices(group1.size());
        std::iota(indices.begin(), indices.end(), 0);
        return std::any_of(indices.begin(), indices.end(), [&group1, &group2](int index) {
            auto &instance1 = group1[index];
            auto &instance2 = group2[index];
            return instance1->not_equal_to(instance2.get());
        });
    }
    return true;
}

void Genome::allocate_memory(const GNPConfig &config)
{
    auto num_genes = 1 + config.num_category_judgement_nodes + config.num_numeric_judgement_nodes + config.num_processing_nodes; // '1' means initial node.
    this->genes.clear();
    this->genes.reserve(num_genes);
    {
        auto index = this->genes.size();
        this->genes.emplace_back(new InitialNodeGene(this, index, config));
    }
    for (int i = 0; i < config.num_category_judgement_nodes; i++)
    {
        auto index = this->genes.size();
        this->genes.emplace_back(new CategoryJudgementNodeGene(this, index, config));
    }
    for (int i = 0; i < config.num_numeric_judgement_nodes; i++)
    {
        auto index = this->genes.size();
        this->genes.emplace_back(new NumericJudgementNodeGene(this, index, config));
    }
    for (int i = 0; i < config.num_processing_nodes; i++)
    {
        auto index = this->genes.size();
        this->genes.emplace_back(new ProcessingNodeGene(this, index, config));
    }
}
}
