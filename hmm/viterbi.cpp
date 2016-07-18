//
// viterbiを計算するスクリプト
// ## 仮定
// * タグは直前のタグにのみ依存する
// * 単語の出現する確率はそのタグにのみ依存する
//
// transition probability, observation likelihoodをlaplace smoothingする
//
////////////////////// 実行例 ////////////////////////////
// $ cat test.txt | ./test2sentence | ./viterbi n_tag_trans_count.txt n-1_tag_trans_count.txt obs_likli.txt
// NN IN DT NN VBZ RB VBN TO VB DT JJ NN IN NN NNS IN NNP , JJ IN NN NN , VB TO VB DT JJ NN IN NNP CC NNP POS JJ NNS .
// NNP IN DT NNP NNP NNP POS VBN NN TO DT NN JJ NN VBZ VBN TO VB DT NN IN NN IN DT JJ NN .
// ...
// `` DT NNS IN NN IN DT JJ NN NN VBP RB RB IN DT JJ NN , '' VBD NNP NNP , JJ NNP NN IN NNP NNP NNP .
// precision: 0.90
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
#include <unordered_map>
#include <unordered_set>
#include <cmath>

void calcTransitionLogProbability(const std::string& ntag_trans_count_file,
                                  const std::string& n_1tag_trans_count_file,
                                  const std::unordered_set<std::string>& unique_tags,
                                std::unordered_map<std::string, double>* transition_logp_map);
void getUniqueTags(const std::string& n_1tag_trans_count_file, std::unordered_set<std::string>* unique_tags);


int main(int argc, char** argv)
{
    if (argc < 4) {
        std::cerr << "Usage example: cat test.txt | ./test2sentence | " << argv[0]
        << "n_tag_transition_count.txt n-1_tag_transition_count.txt observation_likelihood.txt" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string ntag_trans_count_file = argv[1];
    std::string n_1tag_trans_count_file = argv[2];
    std::string observation_likelihood_file = argv[3];

    // ファイル確認
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
    std::ifstream obs_likeli_input(observation_likelihood_file);
    if (!obs_likeli_input) {
        std::cerr << "cannot find observation_likelihood file." << std::endl;
        std::exit(EXIT_FAILURE);
    }


    // 以降で使う各変数の生成
    std::unordered_map<std::string, double> transition_logp_map;
    std::unordered_set<std::string> unique_tags;
    getUniqueTags(n_1tag_trans_count_file, &unique_tags);
    int nof_tags = unique_tags.size();
    calcTransitionLogProbability(ntag_trans_count_file, n_1tag_trans_count_file, unique_tags, &transition_logp_map);

    // testファイルの各行でloopを回す
    char char_test_line[1 << 21];
    while (fgets(char_test_line, 1 << 21, stdin))
    {
        std::string test_line = char_test_line;

        // test lineの各単語でループ（各単語でタグを予測）
        size_t found_pos = 0;
        std::string prev_tag = TAG_START_SYMBOL;
        double viterbi = 1.0;
        while (found_pos != std::string::npos)
        {
            // 単語を抽出
            size_t first_char_pos = (found_pos == 0) ? found_pos : found_pos + 1;
            size_t word_end_pos = test_line.find(SENTENCE_DELIMITER, first_char_pos);
            int word_len = word_end_pos - first_char_pos;
            std::string word = test_line.substr(first_char_pos, word_len);
            if (word == "" || word == "\n") break; // 文末処理

            // 学習時において、単語に対するタグとそのタグの件数の組をobservation_likelihood.txtから取得
            std::unordered_map<std::string, int> obs_tag_count_map;
            int total_obs_tag_count = 0;
            std::string obs_line;
            std::ifstream obs_likeli_input(observation_likelihood_file);
            while (std::getline(obs_likeli_input, obs_line))
            {
                size_t obs_first_delim_pos = obs_line.find(TRAIN_DELIMITER);
                size_t obs_second_delim_pos = obs_line.find(TRAIN_DELIMITER, obs_first_delim_pos + 1);
                std::string obs_word = obs_line.substr(obs_second_delim_pos + 1);
                if (word != obs_word) continue;

                int obs_tag_len = obs_second_delim_pos - obs_first_delim_pos - 1;
                int obs_count = std::stoi(obs_line.substr(0, obs_first_delim_pos));
                std::string obs_tag = obs_line.substr(obs_first_delim_pos + 1, obs_tag_len);

                // カウント
                obs_tag_count_map[obs_tag] = obs_count;
                total_obs_tag_count += obs_count;
            }

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
                int denom = total_obs_tag_count + nof_tags;
                double obs_logp = log((double) numer / denom);
                double viterbi_candidate_logp = log(viterbi) + transition_logp + obs_logp;
                if (vmax <= viterbi_candidate_logp)
                {
                    tag_selected = unique_tag;
                    vmax = viterbi_candidate_logp;
                }
            }
            prev_tag = tag_selected;
            viterbi = exp(vmax);
            std::cout << tag_selected << SENTENCE_DELIMITER;

            // 次のdelimを取得する(次の単語を取得するため)
            found_pos = test_line.find(SENTENCE_DELIMITER, first_char_pos);
        }
        std::cout << std::endl;
    }
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
        size_t first_delim_pos = n_1tag_line.find(TRAIN_DELIMITER);
        size_t second_delim_pos = n_1tag_line.find(TRAIN_DELIMITER, first_delim_pos + 1);
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
        size_t first_delim_pos = n_1tag_line.find(TRAIN_DELIMITER);
        std::string n_1tagseq = n_1tag_line.substr(first_delim_pos + 1);
        n_1tag_count_map[n_1tagseq] = std::stoi(n_1tag_line.substr(0, first_delim_pos));
    }

    // nタグのカウントを記述しているファイルで同様にループ
    std::string ntag_line;
    std::unordered_map<std::string, int> ntag_count_map;
    while (std::getline(ntag_trans_count_input, ntag_line))
    {
        size_t first_delim_pos = ntag_line.find(TRAIN_DELIMITER);
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
