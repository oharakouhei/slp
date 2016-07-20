# Practices based on SLP
Labの輪講で読んだ [SPEECH and LANGUAGE PROCESSING](https://www.cs.colorado.edu/~martin/slp.html) の各章で出てきたアルゴリズムなどについて実装するリポジトリです

## 各ディレクトリについて
### edit_distance
**Minimum Edit Distance (最小編集距離)**

単語対を受け取り最小編集距離を出力する。

### n_gram
**N-Gramモデル**

学習用コーパスから言語モデルを学習し、テストコーパスで perplexity で評価。
N,スムージング手法,データサイズ,データ期間などを変えてみる。 (スムージングはラプラススムージングのみ実装)

### hmm
**Hidden Markov Model**
* 品詞タグ付与の訓練コーパスで訓練し、最尤推定によりモデルパラメータを学習
* Forwardアルゴリズム評価データの尤度を求める
* Viterbiアルゴリズムを評価データに適用して解析精度を計る
    * Backpointer を実装して最尤の系列を出力し, 正解を比較
    * よく混同する品詞対を調べる

## 注意
コーパスやデータはこのリポジトリには置いていません。。