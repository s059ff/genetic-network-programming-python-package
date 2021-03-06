# genetic-network-programming-python-package
遺伝的ネットワークプログラミングのC++実装です。Pythonで利用可能なモジュールを作成します。

## 動作環境
* Ubuntu 16.04 LTS 64bit
* Anaconda 4.5.0以上
* Python 3.6.4以上
* C++14に対応したコンパイラが必要です。
* AnacondaでインストールしたPython 3.x系が対象です。

## 外部ライブラリ
外部ライブラリとして、
* C++ <--> Pythonの 相互変換に必要 'boost'
* 数値計算ライブラリ 'Eigen'
* JSONパーサライブラリ 'picojson'

を必要とします。  

## ビルド方法
1. 'Eigen'と'picojson'をインターネットからダウンロードして、適当なディレクトリに配置します。
2. ターミナルで`conda install boost`を実行します。Anacondaに'boost'がインストールされます。
3. Makefileの以下の点を環境に合わせて修正します。
* EIGEN_PATH := ('Eigen'をインストールしたディレクトリ)
* PICOJSON_PATH := (''picojson'をインストールしたディレクトリ)
* ANACONDA_PATH := ('Anaconda'をインストールしたディレクトリ)
* CC := (C++のコンパイラ、g++やclang++など)

EIGEN_PATH/Eigen/...  
PICOJSON_PATH/picojson.h  
ANACONDA_PATH/bin/...  
となるようにしてください。  
4. ターミナルでgenetic-network-programming-python-package/に移動して、make を実行します。  
gnp.soが出力されます。

以下の設定はオプションです。
* 最適化を有効にする場合  
BUILD_TYPEにRELEASEを設定します。  
最適化を有効にすると、一部のエラーチェックが無効になります。
* OpenMPによるマルチスレッドを有効にする場合  
ENABLE_OPENMPにTRUEを設定し、OMP_NUM_THREADSにスレッド数(16など)を指定します。
* 倍精度浮動小数点数を使用する場合  
GNP_USE_DOUBLE_PRECISIONにTRUEを設定します。

## サンプルプログラム
examples/にサンプルプログラムがあります。
* classification-iris  
Irisデータセットを使ってクラス分類を行います。
main.pyを実行すると、results/に結果が出力されます。
* regression-abalone  
abaloneデータセットを使って回帰分析を行います。  
main.pyを実行すると、results/に結果が出力されます。(精度は悪いです)
* serialization  
GenomeとPopulationのシリアライゼーション・デシリアライゼーションのテストを行います。  
main.pyを実行し、OKと表示されることを確認してください。
* visualization  
Genomeの可視化テストを行います。
main.pyを実行し、dotファイルとpngファイルが出力されることを確認してください。
