#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "DataAttribute.h"
#include "DataAttributeCollection.h"
#include "GNPTypes.h"

namespace gnp
{
class GNPConfig
{
  public:
    // JSON 形式のファイルを読み取り、新規に GNPConfig を作成します。
    GNPConfig(const char *path);

    std::string to_string() const;

  public:
    /**
     * ネットワークの入力に関する設定です。
     **/
    DataAttributeCollection input_attributes;

    /**
     *  ネットワークの出力に関する設定です。
     **/
    DataAttributeCollection output_attributes;

    /**
     *  遺伝的アルゴリズムに関する設定です。
     **/
    // エリート個体を除く個体の総数です。
    int num_genomes;

    // エリート個体の総数です。
    int num_elites;

    // ネットワークを構成するカテゴリ属性の判定ノードの数です。
    int num_category_judgement_nodes;

    // ネットワークを構成する数値属性の判定ノードの数です。
    int num_numeric_judgement_nodes;

    // ネットワークを構成する処理ノードの数です。
    int num_processing_nodes;

    // 判定ノードにおける分岐の数です。
    int num_branches;

    // 全個体のうち交叉を行う個体の割合です。
    double crossover_rate;

    // 判定ノードにおける分岐の突然変異率です。
    double branch_mutation_rate;

    // 判定ノードの入力データの参照先の突然変異率です。
    double data_source_mutation_rate;

    // 判定ノードにおける判定関数の内部パラメータの突然変異率です。
    double judgement_function_mutation_rate;

    // 処理ノードにおける出力値の突然変異率です。
    double output_mutation_rate;

    /**
     *  Genetic Network Programming 固有の設定です。
     **/
    // ネットワークの実行に要する時間コストの上限値です。
    double time_limit;

    // 1 つの処理ノードの実行に要する時間コストです。
    double delay_time_processing_node;

    // 1 つの判定ノードの実行に要する時間コストです。
    double delay_time_judgement_node;
};
}
