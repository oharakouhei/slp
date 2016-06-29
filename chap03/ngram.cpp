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
    std::ifstream input(train_data);
    if (!input) {
        std::cerr << "cannot found training data file!" << std::endl;
    }
    countNgram_(input);
    input.close();
}

double NGram::calcPerplexity(const std::string &testing)
{
    std::cout << "calc" << std::endl;
}


void NGram::showProbabilities()
{
    createN1gramFreq_();

    auto end = ngram_freq_.end();
    for(auto iter = ngram_freq_.begin(); iter != end; iter++)
    {
        double p = probability_(iter->first, iter->second);

        std::vector<std::string> v(iter->first.begin(), iter->first.end()-1);
        std::cout << "P(" << iter->first << "|"
            << v << ") = " << p << std::endl;
    }
}

//
// ngramがそれぞれどれだけ出現するかカウントする
//
void NGram::countNgram_(std::istream &stream)
{
    std::string line;
    initializeTmpNgram_();
    while (std::getline(stream, line))
    {
        std::string word = getSurface_(line);
        updateNgramFreq_(word);
    }
    std::cout << ngram_freq_ << std::endl;
}

//
// 文頭で実行。NGRAM_START_SYMBOLをN-1だけ挿入する
//
void NGram::initializeTmpNgram_()
{
    tmp_ngram_.clear();
    for (int i=0; i < N_; i++)
    {
        updateTmpNgram_(NGRAM_START_SYMBOL);
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
void NGram::updateNgramFreq_(std::string &word)
{
    bool is_sentence_end = FALSE;
    if (word == SENTENCE_END_SYMBOL)
    {
        is_sentence_end = TRUE;
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
    if (is_sentence_end)
    {
        initializeTmpNgram_();
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
// ngramの頻度mapから
// n-1gramの頻度mapを生成
//
void NGram::createN1gramFreq_()
{
    auto end = ngram_freq_.end();
    for(auto iter = ngram_freq_.begin(); iter != end; iter++)
    {
        std::vector<std::string> tmp_n_1gram(iter->first.begin(), iter->first.end()-1);
        n_1gram_freq_[tmp_n_1gram] += iter->second;
    }
    std::cout << n_1gram_freq_ << std::endl;
}

double NGram::probability_(const std::vector<std::string> &ngram, const int &count)
{
    std::vector<std::string> v(ngram.begin(), ngram.end()-1);
    auto iter = n_1gram_freq_.find(v);
    if (iter != n_1gram_freq_.end())
    {
        return (double) count / iter->second;
    }
    else
    {
        return 0;
    }
}