#pragma once

#include <map>
#include <memory>
#include <vector>

#include <picojson.h>

#include "GNPConfig.h"
#include "GNPTypes.h"

namespace gnp
{
class Genome;

// 何らかのノードを表します。
class AbstractNodeGene
{
  public:
    virtual ~AbstractNodeGene() {}

    AbstractNodeGene(const Genome *owner, int index, double delay) : owner(owner), index(index), delay(delay) {}

    virtual const AbstractNodeGene *next(const Vector<data_t> &record) const = 0;

    virtual void mutate(randomizer_t &randomizer, const GNPConfig &config, bool force_mutation = false) = 0;

    virtual std::unique_ptr<AbstractNodeGene> duplicate(const Genome *owner) const = 0;

    virtual void serialize(picojson::object &object, const GNPConfig &config) const;

    virtual void deserialize(const picojson::object &object, const GNPConfig &config);

    virtual bool equal_to(const AbstractNodeGene *other) const;

    virtual bool not_equal_to(const AbstractNodeGene *other) const;

  public:
    // このノードを所有している Genome インスタンス。
    const Genome *owner = nullptr;

    // このノードが格納される配列におけるこのノードのインデックス。
    int index = 0;

    // このノードの実行に要する時間。
    double delay = 0.0;
};

// 遷移開始ノードを表します。
class InitialNodeGene : public AbstractNodeGene
{
    using base = AbstractNodeGene;

  public:
    InitialNodeGene(const Genome *owner, int index, const GNPConfig &config) : base(owner, index, 0.0) {}

    const AbstractNodeGene *next(const Vector<data_t> &record) const override;

    void mutate(randomizer_t &randomizer, const GNPConfig &config, bool force_mutation = false) override;

    std::unique_ptr<AbstractNodeGene> duplicate(const Genome *owner) const override;

    void serialize(picojson::object &object, const GNPConfig &config) const override;

    void deserialize(const picojson::object &object, const GNPConfig &config) override;

    bool equal_to(const AbstractNodeGene *other) const override;

  public:
    // このノードの接続先ノードのインデックス。
    int target;
};

// 処理ノードを表します。
class ProcessingNodeGene : public AbstractNodeGene
{
    using base = AbstractNodeGene;

  public:
    ProcessingNodeGene(const Genome *owner, int index, const GNPConfig &config) : base(owner, index, config.delay_time_processing_node) {}

    const AbstractNodeGene *next(const Vector<data_t> &record) const override;

    void mutate(randomizer_t &randomizer, const GNPConfig &config, bool force_mutation = false) override;

    std::unique_ptr<AbstractNodeGene> duplicate(const Genome *owner) const override;

    void serialize(picojson::object &object, const GNPConfig &config) const override;

    void deserialize(const picojson::object &object, const GNPConfig &config) override;

    bool equal_to(const AbstractNodeGene *other) const override;

  public:
    // このノードの接続先ノードのインデックス。
    int target;

    // このノードの出力値。
    Vector<data_t> value;
};

// 何らかの判定ノードを表します。
class AbstractJudgementNodeGene : public AbstractNodeGene
{
    using base = AbstractNodeGene;

  public:
    AbstractJudgementNodeGene(const Genome *owner, int index, double delay) : base(owner, index, delay) {}

    void mutate(randomizer_t &randomizer, const GNPConfig &config, bool force_mutation = false) override;

    void serialize(picojson::object &object, const GNPConfig &config) const override;

    void deserialize(const picojson::object &object, const GNPConfig &config) override;

    bool equal_to(const AbstractNodeGene *other) const override;

  public:
    // このノードの接続先ノードのインデックス。
    std::vector<int> targets;

    // このノードへ入力する要素のインデックス。
    int source;
};

// カテゴリ属性の判定ノードを表します。
class CategoryJudgementNodeGene : public AbstractJudgementNodeGene
{
    using base = AbstractJudgementNodeGene;

  public:
    CategoryJudgementNodeGene(const Genome *owner, int index, const GNPConfig &config) : base(owner, index, config.delay_time_judgement_node) {}

    const AbstractNodeGene *next(const Vector<data_t> &record) const override;

    void mutate(randomizer_t &randomizer, const GNPConfig &config, bool force_mutation = false) override;

    std::unique_ptr<AbstractNodeGene> duplicate(const Genome *owner) const override;

    void serialize(picojson::object &object, const GNPConfig &config) const override;

    void deserialize(const picojson::object &object, const GNPConfig &config) override;

    bool equal_to(const AbstractNodeGene *other) const override;

  public:
    // カテゴリからブランチのインデックスへの変換関数。
    std::map<category_t, int> branches;
};

// 数値属性の判定ノードを表します。
class NumericJudgementNodeGene : public AbstractJudgementNodeGene
{
    using base = AbstractJudgementNodeGene;

  public:
    NumericJudgementNodeGene(const Genome *owner, int index, const GNPConfig &config) : base(owner, index, config.delay_time_judgement_node) {}

    const AbstractNodeGene *next(const Vector<data_t> &record) const override;

    void mutate(randomizer_t &randomizer, const GNPConfig &config, bool force_mutation = false) override;

    std::unique_ptr<AbstractNodeGene> duplicate(const Genome *owner) const override;

    void serialize(picojson::object &object, const GNPConfig &config) const override;

    void deserialize(const picojson::object &object, const GNPConfig &config) override;

    bool equal_to(const AbstractNodeGene *other) const override;

  public:
    // 数値データの値を分割するしきい値。
    std::vector<numeric_t> thresholds;
};
}
