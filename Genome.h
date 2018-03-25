#pragma once

#include <memory>
#include <vector>

#include <picojson.h>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "GNPConfig.h"
#include "GNPTypes.h"
#include "NodeGene.h"

namespace gnp
{
// 遺伝子を表します。
class Genome
{
  public:
    // ランダムに新しい個体を生成します。
    void configure_new(randomizer_t &randomizer, const GNPConfig &config);

    // 親個体からパラメータを引き継ぎます。
    void configure_inheritance(const Genome &parent);

    // 親個体からパラメータを引き継ぎます。
    void configure_inheritance_move(Genome &&parent);

    // 2 つの親個体を交叉して新しい遺伝子を生成します。
    void configure_crossover(randomizer_t &randomizer, const Genome &parent1, const Genome &parent2);

    // 突然変異を行います。
    void mutate(randomizer_t &randomizer, const GNPConfig &config);

    // 指定されたファイルに個体情報を保存します。
    void serialize(const char *path, const GNPConfig &config) const;

    void serialize_to_object(picojson::object &object, const GNPConfig &config) const;

    // 指定されたファイルから個体情報を復元します。
    void deserialize(const char *path, const GNPConfig &config);

    void deserialize_from_object(const picojson::object &object, const GNPConfig &config);

    // ネットワーク図を画像ファイルに出力します。
    void savefig(const char *path, const GNPConfig &config) const;

    // ノード遷移を行います。
    Matrix<data_t> activate(const Vector<data_t> &vector, const GNPConfig &config) const;

    // ノード遷移を行います。
    boost::python::numpy::ndarray activate_py(boost::python::numpy::ndarray vector, const GNPConfig &config) const;

  public:
    Genome() = default;

    Genome(const Genome &source)
    {
        *this = source;
    }

    Genome(Genome &&source)
    {
        *this = std::move(source);
    }

    Genome &operator=(const Genome &source)
    {
        if (this != &source)
        {
            this->configure_inheritance(source);
        }
        return *this;
    }

    Genome &operator=(Genome &&source)
    {
        if (this != &source)
        {
            this->configure_inheritance_move(std::move(source));
        }
        return *this;
    }

    void configure_new_py(const GNPConfig &config)
    {
        auto randomizer = randomizer_t(std::random_device()());
        this->configure_new(randomizer, config);
    }

    void configure_crossover_py(const Genome &parent1, const Genome &parent2)
    {
        auto randomizer = randomizer_t(std::random_device()());
        this->configure_crossover(randomizer, parent1, parent2);
    }

    void mutate_py(const GNPConfig &config)
    {
        auto randomizer = randomizer_t(std::random_device()());
        this->mutate(randomizer, config);
    }

    bool equal_to(const Genome &other) const;

    bool not_equal_to(const Genome &other) const;

  private:
    void allocate_memory(const GNPConfig &config);

  public:
    // フィットネス値。
    double fitness;

    // ネットワークを構成するノードの集合。
    std::vector<std::unique_ptr<AbstractNodeGene>> genes;

  public:
    friend bool operator==(const Genome &a, const Genome &b)
    {
        return &a == &b;
    }
};
}
