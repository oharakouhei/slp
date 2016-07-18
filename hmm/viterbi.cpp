//
// viterbiを計算するスクリプト
// 最初はobservation likelihoodだけlaplace smoothingするようにする
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
//#include <cmath>

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

    // VS: vocabulary size, VT: vocabulary type, nof_ngrams: number of ngrams
//    int VS, VT, nof_ngrams;
//    countVocabularySize(ngram_file, &VS, &VT, &nof_ngrams);
//    std::cout << "vocabulary: " << VT << ", ";
//    std::cout << N << "-grams: " << nof_ngrams << std::endl;

    // </s>の分で+1
//    int nof_words = countWords(test_file) + 1;

    // testファイルの各行でloopを回す
    char char_test_line[1 << 21];
    while (fgets(char_test_line, 1 << 21, stdin))
    {
        // 文頭シンボルと文末シンボルを追加
        std::string test_line = TAG_START_SYMBOL + SENTENCE_DELIMITER + char_test_line;
        test_line = test_line.replace(test_line.find('\n'), 1, TAG_END_SYMBOL);

        // test lineの各単語でループ（各単語でタグを予測）
        size_t found_pos = 0;
        while (found_pos != std::string::npos)
        {
            // 単語を抽出
            size_t first_char_pos = (found_pos == 0) ? found_pos : found_pos + 1;
            size_t word_end_pos = test_line.find(SENTENCE_DELIMITER, first_char_pos);
            int word_len = word_end_pos - first_char_pos;
            std::string word = test_line.substr(first_char_pos, word_len);

            // 単語が学習時に何件存在しておりまたその時のタグがなんであるかobservation_likelihood.txtから取得
            std::string obs_line;
            std::ifstream obs_likeli_input(observation_likelihood_file);
            while (std::getline(obs_likeli_input, obs_line))
            {
                size_t obs_found = obs_line.find(word);

                size_t obs_first_delim_pos = obs_line.find(TRAIN_DELIMITER);
                size_t obs_second_delim_pos = obs_line.find(TRAIN_DELIMITER, obs_first_delim_pos + 1);
                std::string obs_word = obs_line.substr(obs_second_delim_pos + 1, std::string::npos);
                if (word != obs_word) continue;

                int obs_tag_len = obs_second_delim_pos - obs_first_delim_pos - 1;
                int obs_count = std::stoi(obs_line.substr(0, obs_first_delim_pos));
                std::string obs_tag = obs_line.substr(obs_first_delim_pos + 1, obs_tag_len);

                // 各タグに対して確率を計算
//                vector<double> viterbi_t_vec;
//                viterbi_t_vec;
            }
            // viterbi pathはそれらの確率の最大値
//            viterbi_path.append(viterbi_t_vec.max());


            // 次のdelimを取得する(次の単語を取得するため)
            found_pos = test_line.find(SENTENCE_DELIMITER, first_char_pos);
        }
    }
}
