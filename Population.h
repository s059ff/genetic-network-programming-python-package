#pragma once

#include <random>
#include <vector>

#include "GNPConfig.h"
#include "GNPTypes.h"
#include "Genome.h"

namespace gnp
{
// すべての遺伝子を表します。
class Population
{
  public:
    // このクラスのインスタンスを初期化します。
    Population(const GNPConfig &config);

    // 全個体に対して遺伝子操作を行い、世代を更新します。
    void run(const GNPConfig &config);

    // 指定されたファイルに個体群を保存します。
    void serialize(const char *path, const GNPConfig &config) const;

    // 指定されたファイルから個体群を復元します。
    void deserialize(const char *path, const GNPConfig &config);

    bool equal_to(const Population &other) const;

    bool not_equal_to(const Population &other) const;

  public:
    // 遺伝子の集合。
    std::vector<Genome> genomes;

  private:
#ifndef _OPENMP
    randomizer_t randomizer;
#else
    std::vector<randomizer_t> randomizers;
#endif
};
}
