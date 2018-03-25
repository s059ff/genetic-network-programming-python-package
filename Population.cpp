#include <algorithm>
#include <fstream>
#include <random>
#include <sstream>

#include <omp.h>

#include "Population.h"
#include "runtime_assert.h"

namespace gnp
{
Population::Population(const GNPConfig &config)
{
#ifndef _OPENMP
    this->randomizer = randomizer_t(std::random_device()());
#else
    omp_set_num_threads(OMP_NUM_THREADS);
    this->randomizers.resize(OMP_NUM_THREADS);
    for (auto &randomizer : this->randomizers)
        randomizer = randomizer_t(std::random_device()());
#endif

    this->genomes.clear();
    this->genomes.resize(config.num_genomes);
#pragma omp parallel for
    for (int i = 0; i < this->genomes.size(); i++)
    {
        auto &genome = this->genomes[i];
#ifndef _OPENMP
        auto &randomizer = this->randomizer;
#else
        auto &randomizer = this->randomizers[omp_get_thread_num()];
#endif
        genome.configure_new(randomizer, config);
    }
}

void Population::run(const GNPConfig &config)
{
    auto parents = std::move(this->genomes);
    auto offsprings = std::vector<Genome>();
    offsprings.reserve(config.num_genomes + config.num_elites);

    // 各親個体の選択確率をルーレット選択方式で計算する。
    auto fitnesses = std::vector<double>(parents.size());
    std::for_each(parents.begin(), parents.end(), [](auto &genome) {
        runtime_assert(0.0 <= genome.fitness, "Fitness value is must greater than 0.");
    });
    std::transform(parents.begin(), parents.end(), fitnesses.begin(), [](auto &genome) {
        return genome.fitness;
    });
    runtime_assert(0.0 < std::accumulate(fitnesses.begin(), fitnesses.end(), 0.0), "All fitness values is 0.");
    auto distribution = std::discrete_distribution<int>(fitnesses.begin(), fitnesses.end());

    // 交叉操作を行う。
    {
        auto num_offsprings = static_cast<int>(config.num_genomes * config.crossover_rate);
        auto new_offsprings = std::vector<Genome>(num_offsprings);
#pragma omp parallel for
        for (int i = 0; i < num_offsprings; i++)
        {
#ifndef _OPENMP
            auto &randomizer = this->randomizer;
#else
            auto &randomizer = this->randomizers[omp_get_thread_num()];
#endif
            auto &parent1 = parents[distribution(randomizer)];
            auto &parent2 = parents[distribution(randomizer)];
            Genome &offspring = new_offsprings[i];
            offspring.configure_crossover(randomizer, parent1, parent2);
        }
        auto begin = std::make_move_iterator(new_offsprings.begin());
        auto end = std::make_move_iterator(new_offsprings.end());
        offsprings.insert(offsprings.end(), begin, end);
    }

    // 突然変異操作を行う。
    {
        auto num_offsprings = config.num_genomes - static_cast<int>(config.num_genomes * config.crossover_rate);
        auto new_offsprings = std::vector<Genome>(num_offsprings);
#pragma omp parallel for
        for (int i = 0; i < num_offsprings; i++)
        {
#ifndef _OPENMP
            auto &randomizer = this->randomizer;
#else
            auto &randomizer = this->randomizers[omp_get_thread_num()];
#endif
            auto &parent = parents[distribution(randomizer)];
            Genome &offspring = new_offsprings[i];
            offspring.configure_inheritance(parent);
            offspring.mutate(randomizer, config);
        }
        auto begin = std::make_move_iterator(new_offsprings.begin());
        auto end = std::make_move_iterator(new_offsprings.end());
        offsprings.insert(offsprings.end(), begin, end);
    }

    // エリート個体をコピーする。
    {
        auto num_offsprings = config.num_elites;
        std::nth_element(
            parents.begin(),
            parents.begin() + config.num_elites,
            parents.end(),
            [](auto &parent1, auto &parent2) { return parent1.fitness > parent2.fitness; });
        auto begin = std::make_move_iterator(parents.begin());
        auto end = std::make_move_iterator(parents.begin() + num_offsprings);
        offsprings.insert(offsprings.end(), begin, end);
    }

    // 世代を更新する。
    this->genomes = std::move(offsprings);
}

void Population::serialize(const char *path, const GNPConfig &config) const
{
    picojson::array array;
    array.reserve(this->genomes.size());
    for (auto& genome : this->genomes)
    {
        picojson::object object;
        genome.serialize_to_object(object, config);
        array.push_back(picojson::value(object));
    }

    std::ofstream stream(path);
    stream << picojson::value(array) << std::endl;
}

void Population::deserialize(const char *path, const GNPConfig &config)
{
    std::ifstream stream(path);
    picojson::value value;
    stream >> value;
    auto &array = value.get<picojson::array>();
    this->genomes.clear();
    this->genomes.reserve(array.size());
    for (auto &value : array)
    {
        Genome genome;
        genome.deserialize_from_object(value.get<picojson::object>(), config);
        this->genomes.push_back(std::move(genome));
    }
}

bool Population::equal_to(const Population &other) const
{
    auto &group1 = this->genomes;
    auto &group2 = other.genomes;
    if (group1.size() == group2.size())
    {
        std::vector<int> indices(group1.size());
        std::iota(indices.begin(), indices.end(), 0);
        return std::all_of(indices.begin(), indices.end(), [&group1, &group2](int index) {
            auto &instance1 = group1[index];
            auto &instance2 = group2[index];
            return instance1.equal_to(instance2);
        });
    }
    return false;
}

bool Population::not_equal_to(const Population &other) const
{
    auto &group1 = this->genomes;
    auto &group2 = other.genomes;
    if (group1.size() == group2.size())
    {
        std::vector<int> indices(group1.size());
        std::iota(indices.begin(), indices.end(), 0);
        return std::any_of(indices.begin(), indices.end(), [&group1, &group2](int index) {
            auto &instance1 = group1[index];
            auto &instance2 = group2[index];
            return instance1.not_equal_to(instance2);
        });
    }
    return true;
}
}
