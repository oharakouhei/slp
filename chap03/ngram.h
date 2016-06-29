//
// Created by OharaKohei on 2016/06/28.
//

#ifndef CHAP03_NGRAM_H
#define CHAP03_NGRAM_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include "utility.h"

class NGram
{
private:
    int N_;
    // ngram_freq_のkeyを作成するため、一時的にngramを保持する。
    std::vector<std::string> tmp_ngram_;
    std::map<std::vector<std::string>, int> ngram_freq_;
    // probabilityを計算する為にn-1gramの頻度も計算する必要がある
    std::map<std::vector<std::string>, int> n_1gram_freq_;

    void countNgram_(std::istream &stream);
    void initializeTmpNgram_();
    std::string getSurface_(std::string &line);
    void updateNgramFreq_(std::string &word);
    void updateTmpNgram_(std::string &word);
    void createN1gramFreq_();
    double probability_(const std::vector<std::string> &ngram, const int &count);
public:
    NGram(const int);
    void train(const std::string &train_data);
    double calcPerplexity(const std::string &testing);


    // デバッグ用
    void showProbabilities();
};

#endif //CHAP03_NGRAM_H
