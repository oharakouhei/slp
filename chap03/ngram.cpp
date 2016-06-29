//
// Created by OharaKohei on 2016/06/28.
//
#include <fstream>
#include "ngram.h"

std::string NGRAM_START_SYMBOL = "<s>";
std::string NGRAM_END_SYNBOL = "</s>";
std::string SENTENCE_END_SYMBOL = "EOS";

NGram::NGram(const int N)
{
    N_ = N;
}

void NGram::train(const std::string &train_data)
{
    std::cout << train_data << std::endl;

    std::ifstream input(train_data);

    if (!input) {
        std::cerr << "cannot found training data file!" << std::endl;
    }

    countNgram_(input);
}

void NGram::countNgram_(std::istream &stream)
{
    std::string line;

    updateTmpNgram_(NGRAM_START_SYMBOL);
    while (std::getline(stream, line))
    {
        std::string word = getSurface_(line);
        createNgram_(word);
    }
}

//
// 表層系（単語そのもの）を行から取得する関数
// 例)
// line = "あけおめ	名詞,普通名詞,*,*"
// return "あけおめ"
//
std::string NGram::getSurface_(std::string &line)
{
    std::string delimiter = "\t";
    std::string surface = line.substr(0, line.find(delimiter));
    return surface;
}

//
// ngramのhashを作る関数
// ngramベクトルをkeyにしてその頻度を計算する
//
void NGram::createNgram_(std::string &word)
{
    if (word == SENTENCE_END_SYMBOL)
    {
        word = NGRAM_END_SYNBOL;
    }
    updateTmpNgram_(word);

    if(tmp_ngram_.size() == N_)
    {
        auto iter = ngram_freq_.find(tmp_ngram_);
        if (iter != ngram_freq_.end())
        {
            iter->second++;
        }
        else
        {
            ngram_freq_.emplace(tmp_ngram_, 1);
        }
    }

}

//
// tmp_ngramにwordを保存していく
//
void NGram::updateTmpNgram_(std::string &word)
{
    tmp_ngram_.push_back(word);
    std::cout << tmp_ngram_ << std::endl;
}