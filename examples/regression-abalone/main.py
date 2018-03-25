import datetime
import os

import matplotlib.pylab as plt
import numpy as np
import pydotplus
import pandas as pd
import math

import gnp


def main():

    # 遺伝的ネットワークプログラミングの設定を読み込みます。
    config = gnp.GNPConfig('gnp-config.json')

    # データセットを読み込みます。
    # ('sex' 要素はカテゴリ属性の 'M', 'F', 'I' のいずれかが格納されているため、
    # それぞれ 0, 1, 2 のインデックスに振り分けます。)
    dataframe = pd.read_csv('abalone.data', header=None)
    dataframe.iloc[:, 0] = dataframe.iloc[:, 0].apply(
        lambda label: {'M': 0, 'F': 1, 'I': 2}[label])
    dataframe = dataframe.astype('f')
    inputs = dataframe.values[:, 0:8]
    outputs = dataframe.values[:, 8]

    # (ワーキングディレクトリを移動します。)
    directory = "results"
    os.mkdir(directory) if not os.path.isdir(directory) else None
    os.chdir(directory)
    directory = datetime.datetime.now().strftime("%Y-%m-%d %H.%M.%S.%f")
    os.mkdir(directory)
    os.chdir(directory)

    # popuraion を作成します。
    population = gnp.Population(config)

    # (全世代、全個体の適合度、損失値を格納する変数。)
    fitnesses = []
    losses = []

    # 遺伝的ネットワークプログラミングの学習を行います。
    for generation in range(100):

        # 平均二乗誤差。
        def mean_squared_error(x, y):
            error = 0.
            assert len(x) == len(y)
            for a, b in zip(x, y):
                error += (a - b) ** 2
            return error / len(x)

        # 個体の適合度を計算するために必要な損失値を計算する関数。
        # (処理ノードを通らなかった場合、強制的に '0' を出力したとみなします。)
        def loss(genome):
            estimations = []
            for input, output in zip(inputs, outputs):
                estimation = genome.activate(input, config)
                estimation = float(estimation) if 0 < len(estimation) else 0
                estimations.append(estimation)
            return mean_squared_error(estimations, outputs)

        # 個体の適合度を決定する評価関数。
        # (適合度は、 0~1 の範囲に収まるように、適当にスケーリングします。)
        def evaluate(genome):
            fitness = -math.log(loss(genome) / 10000)
            return fitness

        # 個体の適合度を計算します。
        for genome in population.genomes:
            genome.fitness = evaluate(genome)

        # (全個体の適合度と損失値を変数に保存します。)
        fitnesses.append([])
        losses.append([])
        for genome in population.genomes:
            fitnesses[-1].append(genome.fitness)
            losses[-1].append(loss(genome))

        # (現世代の適合度の最大値や平均値を表示します。)
        best = np.max(fitnesses[-1])
        mean = np.mean(fitnesses[-1])
        variance = np.var(fitnesses[-1])
        print('[%d] best:%f mean:%f variance:%f' %
              (generation, best, mean, variance))

        # 交叉や突然変異などの遺伝的操作を行い、世代を進めます。
        population.run(config)

    # (適合度の推移をグラフ化して保存します。)
    T = len(fitnesses)
    max_fitnesses = [np.max(x) for x in fitnesses]
    mean_fitnesses = [np.mean(x) for x in fitnesses]
    plt.xlabel('Generation')
    plt.ylabel('Fitness')
    plt.xlim([0, T])
    plt.grid()
    plt.plot(np.arange(T), max_fitnesses, label='max')
    plt.plot(np.arange(T), mean_fitnesses, label='mean')
    plt.legend()
    plt.savefig('fitnesses.png')
    plt.close()

    # (平均二乗誤差の推移をグラフ化して保存します。)
    T = len(losses)
    min_losses = [np.min(x) for x in losses]
    mean_losses = [np.mean(x) for x in losses]
    plt.xlabel('Generation')
    plt.ylabel('Loss')
    plt.xlim([0, T])
    plt.grid()
    plt.plot(np.arange(T), min_losses, label='min')
    plt.plot(np.arange(T), mean_losses, label='mean')
    plt.legend()
    plt.savefig('losses.png')
    plt.close()

    # (最良個体をテキスト化して保存します。)
    best = None
    for genome in population.genomes:
        if best is None or best.fitness < genome.fitness:
            best = genome
    best.serialize('best-genome.json', config)

    # (最良個体を画像化して保存します。)
    best.savefig('best-genome.dot', config)
    graph = pydotplus.graphviz.graph_from_dot_file('best-genome.dot')
    graph.write('best-genome.png', format='png')


if __name__ == '__main__':
    os.chdir(os.path.dirname(__file__))
    main()
