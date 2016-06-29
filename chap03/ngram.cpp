//
// Created by OharaKohei on 2016/06/28.
//
#include <fstream>
#include <curses.h>
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
        createNgramFreq_(word);
    }
    std::cout << ngram_freq_ << std::endl;
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
void NGram::createNgramFreq_(std::string &word)
{
    bool is_sentence_end = FALSE;
    if (word == SENTENCE_END_SYMBOL)
    {
        is_sentence_end = TRUE;
        word = NGRAM_END_SYNBOL;
    }
    updateTmpNgram_(word);

    // カウント
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

    if (is_sentence_end)
    {
        clearTmpNgram_();
    }

}

//
// tmp_ngramにwordを保存していく
// 大きさは常にNを保つ
//
void NGram::updateTmpNgram_(std::string &word)
{
    if(tmp_ngram_.size() != N_)
    {
        tmp_ngram_.push_back(word);
    }
    else
    {
        // FIXME: eraseは削除された要素以降のデータがひとつずつ前に移動されるので遅そう。
        tmp_ngram_.erase(tmp_ngram_.begin());
        tmp_ngram_.push_back(word);
    }
}

//
// tmp_ngramを空にする関数
// EOSがきたら呼ぶ
//
void NGram::clearTmpNgram_()
{
    tmp_ngram_.clear();
}