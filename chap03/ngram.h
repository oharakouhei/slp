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

    void countNgram_(std::istream &stream);
    std::string getSurface_(std::string &line);
    void createNgram_(std::string &word);
    void updateTmpNgram_(std::string &word);
public:
    NGram(const int);
    void train(const std::string &train_data);
};

#endif //CHAP03_NGRAM_H
