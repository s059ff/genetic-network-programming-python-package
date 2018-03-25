#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "DataAttribute.h"
#include "DataAttributeCollection.h"
#include "GNPConfig.h"
#include "Genome.h"
#include "NodeGene.h"
#include "Population.h"

using namespace gnp;

namespace py = boost::python;
namespace np = boost::python::numpy;

BOOST_PYTHON_MODULE(gnp)
{
    Py_Initialize();
    np::initialize();

    py::class_<DataAttribute>("DataAttribute")
        .add_property("name", &DataAttribute::get_name)
        .add_property("typename", &DataAttribute::get_typename)
        .add_property("min", &DataAttribute::get_min)
        .add_property("max", &DataAttribute::get_max)
        .add_property("labels", &DataAttribute::get_labels)
        .def("__str__", &DataAttribute::to_string);

    py::class_<DataAttributeCollection>("DataAttributeCollection")
        .def("__str__", &DataAttributeCollection::to_string);

    py::class_<GNPConfig>("GNPConfig", py::init<const char *>())
        .def_readonly("input_attributes", &GNPConfig::input_attributes)
        .def_readonly("output_attributes", &GNPConfig::output_attributes)
        .def_readwrite("num_genomes", &GNPConfig::num_genomes)
        .def_readwrite("num_elites", &GNPConfig::num_elites)
        .def_readonly("num_category_judgement_nodes", &GNPConfig::num_category_judgement_nodes)
        .def_readonly("num_numeric_judgement_nodes", &GNPConfig::num_numeric_judgement_nodes)
        .def_readonly("num_processing_nodes", &GNPConfig::num_processing_nodes)
        .def_readonly("num_branches", &GNPConfig::num_branches)
        .def_readwrite("crossover_rate", &GNPConfig::crossover_rate)
        .def_readwrite("branch_mutation_rate", &GNPConfig::branch_mutation_rate)
        .def_readwrite("data_source_mutation_rate", &GNPConfig::data_source_mutation_rate)
        .def_readwrite("judgement_function_mutation_rate", &GNPConfig::judgement_function_mutation_rate)
        .def_readwrite("output_mutation_rate", &GNPConfig::output_mutation_rate)
        .def_readonly("time_limit", &GNPConfig::time_limit)
        .def_readonly("delay_time_processing_node", &GNPConfig::delay_time_processing_node)
        .def_readonly("delay_time_judgement_node", &GNPConfig::delay_time_judgement_node);

    py::class_<Genome>("Genome")
        .def("configure_new", &Genome::configure_new_py)
        .def("configure_inheritance", &Genome::configure_inheritance)
        .def("configure_crossover", &Genome::configure_crossover_py)
        .def("mutate", &Genome::mutate_py)
        .def("serialize", &Genome::serialize)
        .def("deserialize", &Genome::deserialize)
        .def("savefig", &Genome::savefig)
        .def("activate", &Genome::activate_py)
        .def_readwrite("fitness", &Genome::fitness)
        .def("__eq__", &Genome::equal_to)
        .def("__ne__", &Genome::not_equal_to);

    py::class_<std::vector<Genome>>("std::vector<Genome>")
        .def(py::vector_indexing_suite<std::vector<Genome>>());

    py::class_<Population>("Population", py::init<const GNPConfig &>())
        .def("run", &Population::run)
        .def("serialize", &Population::serialize)
        .def("deserialize", &Population::deserialize)
        .def_readonly("genomes", &Population::genomes)
        .def("__eq__", &Population::equal_to)
        .def("__ne__", &Population::not_equal_to);
}
