# genetic-network-programming-python-package
遺伝的ネットワークプログラミングのC++実装です。Pythonで利用可能なモジュールを作成します。

# 動作環境
* Ubuntu 16.04 LTS 64bit
* Anaconda 4.5.0
* Python 3.6.4
* ビルドには、C++14に対応したコンパイラが必要です。
* AnacondaでインストールしたPython 3.x系が対象です。

# 外部ライブラリ
外部ライブラリとして、
* 数値計算ライブラリ 'Eigen'
* JSONパーサライブラリ 'picojson'
を必要とします。  
これらのライブラリはHeader-onlyなライブラリであるため、ビルドする必要はありません。

# ビルド方法
Makefileの以下の点を環境に合わせて修正します。  
EIGEN_PATH := ('Eigen'をインストールしたディレクトリ)  
PICOJSON_PATH := (''picojson'をインストールしたディレクトリ)  
ANACONDA_PATH := ('Anaconda'をインストールしたディレクトリ)  
EIGEN_PATH/Eigen/...  
PICOJSON_PATH/picojson.h  
ANACONDA_PATH/bin/...  
             /lib/...  
となるようにしてください。  

以下の設定はオプションです。
* 最適化を有効にする場合
BUILD_TYPEにRELEASEを設定します。  
最適化を有効にすると、一部のエラーチェックが無効になります。
* OpenMPによるマルチスレッドを有効にする場合
ENABLE_OPENMPにTRUEを設定し、OMP_NUM_THREADSにスレッド数(16など)を指定します。
* 倍精度浮動小数点数を使用する場合
GNP_USE_DOUBLE_PRECISIONにTRUEを設定します。
