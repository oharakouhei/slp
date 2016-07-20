//
// viterbiを計算するスクリプト
// ## 仮定
// * タグは直前のタグにのみ依存する
// * 単語の出現する確率はそのタグにのみ依存する
//
// transition probability, observation likelihoodをlaplace smoothingする
//
////////////////////// 実行例 ////////////////////////////
// $ cat test.txt | head -50 | ./viterbi n_tag_trans_count.txt n-1_tag_trans_count.txt obs_likli.txt
// NN IN DT NN VBZ RB VBN TO VB DT JJ NN IN NN NNS IN NNP , JJ IN NN NN , VB TO VB DT JJ NN IN NNP CC NNP POS JJ NNS .
// NNP IN DT NNP NNP NNP POS VBN NN TO DT NN JJ NN VBZ VBN TO VB DT NN IN NN IN DT JJ NN .
// DT NN ,
// ...
// `` DT NNS IN NN IN DT JJ NN NN VBP RB RB IN DT JJ NN , '' VBD NNP NNP , JJ NNP NN IN NNP NNP NNP .
// total tags: 47377
// known word tags: 44075, correct known word tags: 39780
// known_word_precision: 0.902552
// unknown word tags: 3302, correct unknown word tags: 944
// unknown_word_precision: 0.285887
//
//////////////////// 入力 (test.txt) /////////////////////
// He PRP B-NP
// is VBZ B-VP
// (He is から始まる一文の他の単語のタグ等の情報)
// deficits NNS I-NP
// . . O
// (改行で文の区切り)
//
///////////////// 出力例 (タグ順とその出現回数) ///////////////
// NN IN ... NNS .
// ...
// precision: 0.90
//
#include "common.h"
#include <fstream>
#include <unordered_set>
#include <cmath>
#include <curses.h>

void calcTransitionLogProbability(const std::string& ntag_trans_count_file,
                                  const std::string& n_1tag_trans_count_file,
                                  const std::unordered_set<std::string>& unique_tags,
                                std::unordered_map<std::string, double>* transition_logp_map);
void getUniqueTags(const std::string& n_1tag_trans_count_file,
                   std::unordered_set<std::string>* unique_tags);
void getWordAndTagByTxt(const std::string& test_line,
                        std::string* test_word,
                        std::string* test_tag);
void getWord2TagsMap(const std::string& obs_likeli_file,
                     std::unordered_map<std::string, std::unordered_map<std::string, int>>* word2tags_map);


int main(int argc, char** argv)
{
    //////////////////////// 実行方法の確認 ////////////////////////////
    if (argc < 4) {
        std::cerr << "Usage example: cat test.txt | ./test2sentence | " << argv[0]
        << "n_tag_transition_count.txt n-1_tag_transition_count.txt observation_likelihood.txt" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string ntag_trans_count_file = argv[1];
    std::string n_1tag_trans_count_file = argv[2];
    std::string obs_likeli_file = argv[3];

    //////////////////////// ファイル確認 ////////////////////////////
    std::ifstream ntag_trans_count_input(ntag_trans_count_file);
    if (!ntag_trans_count_input) {
        std::cerr << "cannot find n_tag_trans_count_file." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::ifstream n_1tag_trans_count_input(n_1tag_trans_count_file);
    if (!n_1tag_trans_count_input) {
        std::cerr << "cannot find n_1tag_trans_count file." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::ifstream obs_likeli_input(obs_likeli_file);
    if (!obs_likeli_input) {
        std::cerr << "cannot find observation_likelihood file." << std::endl;
        std::exit(EXIT_FAILURE);
    }


    ////////////////////////// 以降で使う各変数の生成 ///////////////////////////
    // uniqueなタグのunordered_setを生成
    std::unordered_set<std::string> unique_tags;
    getUniqueTags(n_1tag_trans_count_file, &unique_tags);
    int nof_tags = unique_tags.size();
    // 遷移確率(log形式)を保存したumapを生成
    // {word: log(p), word2: log(p2),...}
    std::unordered_map<std::string, double> transition_logp_map;
    calcTransitionLogProbability(ntag_trans_count_file, n_1tag_trans_count_file, unique_tags, &transition_logp_map);
    // 学習時における、単語とそれに対するタグとその数を保存したumapを生成
    // {word: {tag: count, tag2: count2,..., "total": total_count}, word2:...}
    std::unordered_map<std::string, std::unordered_map<std::string, int>> word2tags_map;
    getWord2TagsMap(obs_likeli_file, &word2tags_map);
    // 精度計算用の変数
    int nof_known_word_tags = 0; // testで出てきたknown wordの総tag数
    int nof_known_word_correct_tags = 0; // known wordのtagの総正解数
    int nof_unknown_word_tags = 0; // testで出てきたunknown wordの総tag数
    int nof_unknown_word_correct_tags = 0; // unknown wordのtagの総正解数


    ////////////////////// test.txtでループここから ///////////////////////////
    // これらの変数は一文の最初で初期化
    size_t found_pos = 0;
    std::string prev_tag = TAG_START_SYMBOL;
    double viterbi = 1.0;
    // ループ
    char char_test_line[1 << 21];
    while (fgets(char_test_line, 1 << 21, stdin))
    {
        // 改行時処理
        if (char_test_line[0] == '\n')
        {
            // 一文の最初で初期化
            std::cout << std::endl;
            found_pos = 0;
            prev_tag = TAG_START_SYMBOL;
            viterbi = 1.0;
            continue;
        }

        /////////// test.txtでの単語とタグを抽出 /////////
        std::string test_line = char_test_line;
        std::string test_word, test_tag;
        getWordAndTagByTxt(test_line, &test_word, &test_tag);
        // if (test_word == "" || test_word == "\n") break; // </s>が出力されないよう文末処理

        ///// 学習時における単語に対するタグとそのタグの件数の組を冒頭で生成したumapから取得 /////
        std::unordered_map<std::string, int> obs_tag_count_map;
        // 既知語フラグ
        bool isKnownWord = FALSE;
        if (!word2tags_map[test_word].empty())
        {
            isKnownWord = TRUE;
            obs_tag_count_map = word2tags_map[test_word];
        }

        ///////////// viterbiの計算 ////////////
        // 各タグに対してviterbiの確率を計算し，最大の要素を選択する
        double vmax = -99999;
        std::string tag_selected;
        for (auto unique_tag : unique_tags)
        {
            std::string tagseq = prev_tag + SENTENCE_DELIMITER + unique_tag;
            // prev_tagとtagを与え、transition_pを計算結果を返す
            double transition_logp = transition_logp_map[tagseq];
            // 分子分母
            int numer = obs_tag_count_map[unique_tag] + 1;
            int denom = obs_tag_count_map["total"] + nof_tags;
            double obs_logp = log((double) numer / denom);
            double viterbi_candidate_logp = log(viterbi) + transition_logp + obs_logp;
            if (vmax <= viterbi_candidate_logp)
            {
                tag_selected = unique_tag;
                vmax = viterbi_candidate_logp;
            }
        }
        // 次回のために変数を更新
        prev_tag = tag_selected;
        viterbi = exp(vmax);
        std::cout << tag_selected << SENTENCE_DELIMITER;

        // 精度の計算用
        if (isKnownWord)
            nof_known_word_tags += 1;
        else
            nof_unknown_word_tags += 1;
        if (test_tag == tag_selected)
        {
            if (isKnownWord)
                nof_known_word_correct_tags += 1;
            else
                nof_unknown_word_correct_tags += 1;
        }
    }
    ////////////////////// test.txtでループここまで ///////////////////////////

    ////////////////////// precisionのアウトプット ///////////////////////////
    double known_word_precision = (double) nof_known_word_correct_tags / nof_known_word_tags;
    double unknown_word_precision = (double) nof_unknown_word_correct_tags / nof_unknown_word_tags;
    std::cout << std::endl << "total tags: " << nof_known_word_tags + nof_unknown_word_tags << std::endl;
    std::cout << "known word tags: " << nof_known_word_tags;
    std::cout << ", correct known word tags: " << nof_known_word_correct_tags << std::endl;
    std::cout << "known_word_precision: " << known_word_precision << std::endl;
    std::cout << "unknown word tags: " << nof_unknown_word_tags;
    std::cout << ", correct unknown word tags: " << nof_unknown_word_correct_tags << std::endl;
    std::cout << "unknown_word_precision: " << unknown_word_precision << std::endl;
}


//
// uniqueなtagの数を数える関数
// @param n_1tag_trans_count_file: タグ順とその出現回数を記述したファイル
// @param unique_tags: uniqueなタグの集合
//
void getUniqueTags(const std::string& n_1tag_trans_count_file, std::unordered_set<std::string> *unique_tags)
{
    std::ifstream n_1tag_trans_count_input(n_1tag_trans_count_file);
    std::string n_1tag_line;
    while (std::getline(n_1tag_trans_count_input, n_1tag_line))
    {
        size_t first_delim_pos = n_1tag_line.find(SENTENCE_DELIMITER);
        size_t second_delim_pos = n_1tag_line.find(SENTENCE_DELIMITER, first_delim_pos + 1);
        int tag_len = second_delim_pos - first_delim_pos - 1;
        std::string tag = n_1tag_line.substr(first_delim_pos + 1, tag_len);
        unique_tags->insert(tag);
    }
}


//
// 遷移確率を計算しmapに格納
// @param ntag_trans_count_file: タグ順とそのタグ順の学習時の出現回数を保存しているファイル
// @param n_1tag_trans_count_file: ntag_trans_count_fileよりも1少ないタグ順とその学習時の出現回数を保存しているファイル
// @param transition_logp_map: タグ順をkeyに，そのタグ順のように遷移する確率をvalueに格納したもの
// 例) key: "WRB VBN", value: -5.03 (WRBからVBNへと遷移する確率)
//
void calcTransitionLogProbability(const std::string& ntag_trans_count_file,
                                  const std::string& n_1tag_trans_count_file,
                                  const std::unordered_set<std::string>& unique_tags,
                                  std::unordered_map<std::string, double> *transition_logp_map)
{
    std::ifstream ntag_trans_count_input(ntag_trans_count_file);
    std::ifstream n_1tag_trans_count_input(n_1tag_trans_count_file);

    // まずn-1タグのカウントを記述しているファイルからタグの回数をカウントしmapに格納する
    // 以下のループ内で何度もループさせると処理が遅そうなので
    std::string n_1tag_line;
    std::unordered_map<std::string, int> n_1tag_count_map;
    while (std::getline(n_1tag_trans_count_input, n_1tag_line))
    {
        size_t first_delim_pos = n_1tag_line.find(SENTENCE_DELIMITER);
        std::string n_1tagseq = n_1tag_line.substr(first_delim_pos + 1);
        n_1tag_count_map[n_1tagseq] = std::stoi(n_1tag_line.substr(0, first_delim_pos));
    }

    // nタグのカウントを記述しているファイルで同様にループ
    std::string ntag_line;
    std::unordered_map<std::string, int> ntag_count_map;
    while (std::getline(ntag_trans_count_input, ntag_line))
    {
        size_t first_delim_pos = ntag_line.find(SENTENCE_DELIMITER);
        std::string ntagseq = ntag_line.substr(first_delim_pos + 1);
        ntag_count_map[ntagseq] = std::stoi(ntag_line.substr(0, first_delim_pos));
    }

    // 遷移確率pのlog
    // n_1tag_count_map、n_1tag_count_mapを利用
    for (auto first_tag : unique_tags)
    {
        for (auto second_tag : unique_tags)
        {
            std::string tagseq = first_tag + SENTENCE_DELIMITER + second_tag;
            // 分子
            int numer = ntag_count_map[tagseq] + 1;
            // 分母
            int denom = n_1tag_count_map[first_tag] + unique_tags.size();
            // 遷移確率pのlog
            transition_logp_map->emplace(tagseq, log((double) numer / denom));
        }
    }
}


//
// 解析済みファイルからwordとtagを抽出するスクリプト
//
void getWordAndTagByTxt(const std::string& line, std::string* word, std::string* tag)
{
    size_t first_delim_pos = line.find(TXT_DELIMITER);
    size_t second_delim_pos = line.find(TXT_DELIMITER, first_delim_pos + 1);
    int tag_len = second_delim_pos - first_delim_pos - 1;
    *word = line.substr(0, line.find(SENTENCE_DELIMITER));
    *tag = line.substr(first_delim_pos + 1, tag_len);

}

//
// 学習時における、単語とそれに対するタグとその数をumapに保存。また、単語におけるタグの総数も保存しておく。
// @param obs_likeli_file: observation likelihoodを計算済みのファイル名
// @param word2tags_map: {word: {tag: count, tag2: count2,..., "total": total_count}, word2:...}
// という形式のumap
//
void getWord2TagsMap(const std::string& obs_likeli_file,
                     std::unordered_map<std::string, std::unordered_map<std::string, int>>* word2tags_map)
{
    std::string line;
    std::ifstream likeli_input(obs_likeli_file);
    while (std::getline(likeli_input, line))
    {
        // カウント・タグ・単語を取得
        size_t first_delim_pos = line.find(SENTENCE_DELIMITER);
        size_t second_delim_pos = line.find(SENTENCE_DELIMITER, first_delim_pos + 1);
        int tag_len = second_delim_pos - first_delim_pos - 1;
        int count = std::stoi(line.substr(0, first_delim_pos));
        std::string tag = line.substr(first_delim_pos + 1, tag_len);
        std::string word = line.substr(second_delim_pos + 1);

        // カウントのpairを作成
        std::pair<std::string, int> tag_count_pair(tag, count);

        auto iter = word2tags_map->find(word);
        if (iter != word2tags_map->end())
        {
            iter->second["total"] += count;
            iter->second.insert(tag_count_pair);
        }
        else
        {
            std::unordered_map<std::string, int> tag_count_umap = {tag_count_pair, {"total", count}};
            word2tags_map->emplace(word, tag_count_umap);
        }
    }

}