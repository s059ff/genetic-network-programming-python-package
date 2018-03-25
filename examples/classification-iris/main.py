import datetime
import os

import matplotlib.pylab as plt
import numpy as np
import pydotplus
from sklearn import datasets

import gnp


def main():

    # 遺伝的ネットワークプログラミングの設定を読み込みます。
    config = gnp.GNPConfig('gnp-config.json')

    # データセットを読み込みます。
    dataset = datasets.load_iris()
    inputs = dataset.data
    outputs = dataset.target

    # (ワーキングディレクトリを移動します。)
    directory = "results"
    os.mkdir(directory) if not os.path.isdir(directory) else None
    os.chdir(directory)
    directory = datetime.datetime.now().strftime("%Y-%m-%d %H.%M.%S.%f")
    os.mkdir(directory)
    os.chdir(directory)

    # popuraion を作成します。
    population = gnp.Population(config)

    # (全世代、全個体の適合度を格納する変数。)
    fitnesses = []

    # 遺伝的ネットワークプログラミングの学習を行います。
    for generation in range(100):

        # 個体の適合度を決定する評価関数。
        def evaluate(genome):
            num_corrects = 0
            for input, output in zip(inputs, outputs):
                estimation = genome.activate(input, config)
                if 0 < len(estimation):
                    if float(estimation) == output:
                        num_corrects += 1
            accuracy = num_corrects / len(outputs)
            return accuracy

        # 個体の適合度を計算します。
        for genome in population.genomes:
            genome.fitness = evaluate(genome)

        # (全個体の適合度を変数に保存します。)
        fitnesses.append([])
        for genome in population.genomes:
            fitnesses[-1].append(genome.fitness)

        # (現世代の適合度の最大値や平均値を表示します。)
        best = np.max(fitnesses[-1])
        mean = np.mean(fitnesses[-1])
        variance = np.var(fitnesses[-1])
        print('[%d] best:%f mean:%f variance:%f' % (generation, best, mean, variance))

        # 交叉や突然変異などの遺伝的操作を行い、世代を進めます。
        population.run(config)

    # (適合度の推移をグラフ化して保存します。)
    T = len(fitnesses)
    max_fitnesses = [np.max(x) for x in fitnesses]
    mean_fitnesses = [np.mean(x) for x in fitnesses]
    plt.xlabel('Generation')
    plt.ylabel('Fitness')
    plt.xlim([0, T])
    plt.ylim([0, 1])
    plt.grid()
    plt.plot(np.arange(T), max_fitnesses, label='max')
    plt.plot(np.arange(T), mean_fitnesses, label='mean')
    plt.legend()
    plt.savefig('fitnesses.png')
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
