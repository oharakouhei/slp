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
#include <cmath>
#include <curses.h>

void calcTransitionLogProbability(const std::string& ntag_trans_count_file,
                                  const std::string& n_1tag_trans_count_file,
                                  const std::vector<std::string>& unique_tags,
                                std::unordered_map<std::string, double>* transition_logp_map);
void getUniqueTags(const std::string& n_1tag_trans_count_file,
                   std::vector<std::string>* unique_tags);
bool getLineWordsAndTags(std::vector<std::string>* test_words,
                         std::vector<std::string>* test_tags);
void getTag2WordsMap(const std::string& obs_likeli_file,
                     int* vocab_size,
                     std::unordered_map<std::string, std::unordered_map<std::string, int>>* tag2words_map);
void outputConfusedMatrix(const std::vector<std::string>& unique_tags,
                          const std::unordered_map<std::string, std::unordered_map<std::string, int>>& confused_matrix);
double calcLogObservationLikelihood(const int& vocab_size,
                                 const std::unordered_map<std::string, int>& obs_word_count_map,
                                 const std::string& word);

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
    // uniqueなタグのvectorを生成
    std::vector<std::string> unique_tags;
    getUniqueTags(n_1tag_trans_count_file, &unique_tags);
    const int nof_tags = unique_tags.size();
    // 遷移確率(log形式)を保存したumapを生成
    // {word: log(p), word2: log(p2),...}
    std::unordered_map<std::string, double> transition_logp_map;
    calcTransitionLogProbability(ntag_trans_count_file, n_1tag_trans_count_file, unique_tags, &transition_logp_map);
    // 学習時における、単語とそれに対するタグとその数を保存したumapを生成
    // {tag: {word: count, word2: count2,..., "total_count": total_count}, tag2:...}
    std::unordered_map<std::string, std::unordered_map<std::string, int>> tag2words_map;
    int vocab_size = 0;
    getTag2WordsMap(obs_likeli_file, &vocab_size, &tag2words_map);
    // 精度計算用の変数
    int nof_known_word_tags = 0; // testで出てきたknown wordの総tag数
    int nof_known_word_correct_tags = 0; // known wordのtagの総正解数
    int nof_unknown_word_tags = 0; // testで出てきたunknown wordの総tag数
    int nof_unknown_word_correct_tags = 0; // unknown wordのtagの総正解数
    // confused matrix
    std::unordered_map<std::string, std::unordered_map<std::string, int>> confused_matrix;

    ////////////////////// testここから ///////////////////////////
    std::vector<std::string> test_tags;
    std::vector<std::string> test_words;
    /////////////////// 行でループここから ///////////////////////
    while (getLineWordsAndTags(&test_words, &test_tags)) {
        int line_len = test_words.size();
        // これらの変数は一文の最初で初期化
        std::vector<double> prev_logp_vec(nof_tags);
        size_t found_pos = 0;
        double viterbi_logp = 0.0;
        bool isFirstWord = TRUE;
        std::vector<std::vector<int>> bp; // backpointer
        std::vector<bool> is_known_word_vec; // 単語が既知語かどうかのvec


        ///////////////// 単語でループここから ////////////////////
        for (int t = 0; t < line_len; t++) {
            std::string test_word = test_words.at(t);
            std::string test_tag = test_tags.at(t);
            bool isKnownWord = FALSE;

            /// 学習時における単語に対するタグとそのタグの件数の組を冒頭で生成したumapから参照するumap
            std::unordered_map<std::string, int> obs_tag_count_map;

            ///////////// viterbiの計算 ////////////
            // 初回は1回のループで良い
            if (isFirstWord) {
                std::string prev_tag = TAG_START_SYMBOL;
                for (int i = 0; i < nof_tags; i++) {
                    std::string current_tag = unique_tags.at(i);
                    // transition probability
                    std::string tagseq = prev_tag + SENTENCE_DELIMITER + current_tag;
                    double transition_logp = transition_logp_map.at(tagseq);
                    // opservation probability
                    const std::unordered_map<std::string, int>& obs_word_count_map = tag2words_map.at(current_tag);
                    double obs_logp = calcLogObservationLikelihood(vocab_size, obs_word_count_map, test_word);
                    double logp = 0.0 + transition_logp + obs_logp;
                    prev_logp_vec.at(i) = logp;
                    // sum
                    // observation_p_map[current_tag] += exp(total_logp);

                    // 既知語判定
                    if (!isKnownWord) {
                        auto owcm_iter = obs_word_count_map.find(test_word);
                        if (owcm_iter != obs_word_count_map.end())
                            isKnownWord = TRUE;
                    }
                }
                isFirstWord = FALSE;
            }
            else {
                // 2回目以降は全てのタグでループ
                std::vector<double> max_logp_vec(nof_tags);
                std::vector<int> max_index_vec(nof_tags);
                for (int i = 0; i < nof_tags; i++) {
                    std::string current_tag = unique_tags.at(i);
                    // opservation probability
                    const std::unordered_map<std::string, int>& obs_word_count_map = tag2words_map.at(current_tag);
                    double obs_logp = calcLogObservationLikelihood(vocab_size, obs_word_count_map, test_word);
                    double max_log_prob = -9999;
                    int max_index = 0;

                    for (int j = 0; j < nof_tags; j++) {
                        std::string prev_tag = unique_tags.at(j);
                        // transition probability
                        std::string tagseq = prev_tag + SENTENCE_DELIMITER + current_tag;
                        double transition_logp = transition_logp_map.at(tagseq);
                        double logp = prev_logp_vec.at(j) + transition_logp + obs_logp;
                        if (max_log_prob <= logp)
                            max_log_prob = logp, max_index = j;
                    }

                    // 既知語判定
                    if (!isKnownWord) {
                        auto owcm_iter = obs_word_count_map.find(test_word);
                        if (owcm_iter != obs_word_count_map.end())
                            isKnownWord = TRUE;
                    }
                    max_logp_vec.at(i) = max_log_prob;
                    max_index_vec.at(i) = max_index;
                }
                prev_logp_vec = max_logp_vec;
                bp.push_back(max_index_vec);
            }
            is_known_word_vec.push_back(isKnownWord);
        }
        ///////////////// 単語でループここまで ////////////////////
        // 改行時処理
        // 各タグに対してviterbiの確率を計算し，最大の要素を選択する
        // 既知語フラグ
        std::string current_tag = TAG_END_SYMBOL;
        double max_log_prob = -9999;
        int max_index;
        for (int j = 0; j < nof_tags; j++) {
            std::string prev_tag = unique_tags.at(j);
            // transition probability
            std::string tagseq = prev_tag + SENTENCE_DELIMITER + current_tag;
            double transition_logp = transition_logp_map.at(tagseq);
            double logp = prev_logp_vec.at(j) + transition_logp + 0.0;
            if (max_log_prob <= logp) {
                max_log_prob = logp, max_index = j;
            }
        }
        // max_indexからbpを遡る
        int prev_id = -1;
        std::vector<int> outputs = { max_index };
        for (int k = bp.size() - 1; k >= 0; k--) {
            if (prev_id == -1) {
                prev_id = bp.at(k).at(max_index);
                outputs.push_back(prev_id);
            }
            else {
                prev_id = bp.at(k).at(prev_id);
                outputs.push_back(prev_id);
            }
        }
        int outputs_end = outputs.size() - 1;
        for (int k = outputs_end; k >= 0; k--) {
            std::string pred_tag = unique_tags.at(outputs.at(k));
            std::cout << pred_tag << SENTENCE_DELIMITER;

            // 精度の計算用
            if (is_known_word_vec[k])
                nof_known_word_tags += 1;
            else
                nof_unknown_word_tags += 1;
            std::string ans = test_tags.at(outputs_end - k);
            if (ans == pred_tag) {
                if (is_known_word_vec[k])
                    nof_known_word_correct_tags += 1;
                else
                    nof_unknown_word_correct_tags += 1;
            }
            else {
                // カウントのpairを作成
                std::pair<std::string, int> tmp_pair(ans, 1);
                auto iter = confused_matrix.find(pred_tag);
                if (iter != confused_matrix.end()) {
                    iter->second["total_count"] += 1;
                    iter->second.insert(tmp_pair);
                }
                else {
                    std::unordered_map<std::string, int> tmp_umap = {tmp_pair, {"total_count", 1}};
                    confused_matrix.emplace(pred_tag, tmp_umap);
                }
            }
        }
        std::cout << std::endl;
    }
    /////////////////// 行でループここまで ///////////////////////
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

    ////////////////////// confused matrixのアウトプット /////////////////////
//    outputConfusedMatrix(unique_tags, confused_matrix);
}


//
// uniqueなtagの数を数える関数
// @param n_1tag_trans_count_file: タグ順とその出現回数を記述したファイル
// @param unique_tags: uniqueなタグの集合
//
void getUniqueTags(const std::string& n_1tag_trans_count_file, std::vector<std::string> *unique_tags)
{
    std::ifstream input(n_1tag_trans_count_file);
    std::string line;
    while (std::getline(input, line)) {
        size_t first_delim_pos = line.find(SENTENCE_DELIMITER);
        size_t second_delim_pos = line.find(SENTENCE_DELIMITER, first_delim_pos + 1);
        int tag_len = second_delim_pos - first_delim_pos - 1;
        std::string tag = line.substr(first_delim_pos + 1, tag_len);
        if (tag != TAG_START_SYMBOL && tag != TAG_END_SYMBOL) {
            auto end = unique_tags->end();
            auto iter = std::find(unique_tags->begin(), end, tag);
            if (iter == end)
                unique_tags->push_back(tag);
        }

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
                                  const std::vector<std::string>& unique_tags,
                                  std::unordered_map<std::string, double> *transition_logp_map)
{
    std::ifstream ntag_trans_count_input(ntag_trans_count_file);
    std::ifstream n_1tag_trans_count_input(n_1tag_trans_count_file);

    // まずn-1タグのカウントを記述しているファイルからタグの回数をカウントしmapに格納する
    // 以下のループ内で何度もループさせると処理が遅そうなので
    std::string n_1tag_line;
    std::unordered_map<std::string, int> n_1tag_count_map;
    while (std::getline(n_1tag_trans_count_input, n_1tag_line)) {
        size_t first_delim_pos = n_1tag_line.find(SENTENCE_DELIMITER);
        std::string n_1tagseq = n_1tag_line.substr(first_delim_pos + 1);
        n_1tag_count_map[n_1tagseq] = std::stoi(n_1tag_line.substr(0, first_delim_pos));
    }

    // nタグのカウントを記述しているファイルで同様にループ
    std::string ntag_line;
    std::unordered_map<std::string, int> ntag_count_map;
    while (std::getline(ntag_trans_count_input, ntag_line)) {
        size_t first_delim_pos = ntag_line.find(SENTENCE_DELIMITER);
        std::string ntagseq = ntag_line.substr(first_delim_pos + 1);
        ntag_count_map[ntagseq] = std::stoi(ntag_line.substr(0, first_delim_pos));
    }

    std::vector<std::string> first_tags, second_tags;
    first_tags = unique_tags, second_tags = unique_tags;
    first_tags.push_back(TAG_START_SYMBOL);
    second_tags.push_back(TAG_END_SYMBOL);
    // 遷移確率pのlog
    // n_1tag_count_map、n_1tag_count_mapを利用
    for (auto first_tag : first_tags) {
        for (auto second_tag : second_tags) {
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
// 一文の単語とタグをvecotrに変える関数
// 文が存在していればTRUEを返し、なければFALSEを返す
//
bool getLineWordsAndTags(std::vector<std::string>* test_words, std::vector<std::string>* test_tags)
{
    *test_words = {};
    *test_tags = {};
    char char_line[1 << 21];
    while (fgets(char_line, 1 << 21, stdin)) {
        if (char_line[0] == '\n') return TRUE;
        /////////// test.txtでの単語とタグを抽出 /////////
        std::string line = char_line;
        size_t first_delim_pos = line.find(TXT_DELIMITER);
        size_t second_delim_pos = line.find(TXT_DELIMITER, first_delim_pos + 1);
        int tag_len = second_delim_pos - first_delim_pos - 1;
        std::string word, tag;
        word = line.substr(0, line.find(SENTENCE_DELIMITER));
        tag = line.substr(first_delim_pos + 1, tag_len);
        test_words->push_back(word);
        test_tags->push_back(tag);
    }
    return FALSE;
}


//
// 学習時における、単語とそれに対するタグとその数をumapに保存。また、単語におけるタグの総数も保存しておく。
// @param obs_likeli_file: observation likelihoodを計算済みのファイル名
// @param vocab_size: 学習時の総単語数
// @param tag2words_map: {tag: {word: count, word2: count2,..., "total_count": total_count}, tag2:...}
// という形式のumap
//
void getTag2WordsMap(const std::string& obs_likeli_file,
                     int* vocab_size,
                     std::unordered_map<std::string, std::unordered_map<std::string, int>>* tag2words_map)
{
    std::string line;
    std::ifstream likeli_input(obs_likeli_file);
    while (std::getline(likeli_input, line)) {
        // カウント・タグ・単語を取得
        size_t first_delim_pos = line.find(SENTENCE_DELIMITER);
        size_t second_delim_pos = line.find(SENTENCE_DELIMITER, first_delim_pos + 1);
        int tag_len = second_delim_pos - first_delim_pos - 1;
        int count = std::stoi(line.substr(0, first_delim_pos));
        std::string tag = line.substr(first_delim_pos + 1, tag_len);
        std::string word = line.substr(second_delim_pos + 1);

        // vocab数を
        *vocab_size += count;

        // カウントのpairを作成
        std::pair<std::string, int> word_count_pair(word, count);

        auto iter = tag2words_map->find(tag);
        if (iter != tag2words_map->end()) {
            iter->second["total_count"] += count;
            iter->second.insert(word_count_pair);
        }
        else {
            std::unordered_map<std::string, int> word_count_umap = {word_count_pair, {"total_count", count}};
            tag2words_map->emplace(tag, word_count_umap);
        }
    }

}

//
// confused matrix出力関数
//
void outputConfusedMatrix(const std::vector<std::string>& unique_tags,
                          const std::unordered_map<std::string, std::unordered_map<std::string, int>>& confused_matrix)
{
    std::cout << "   ";
    for (auto unique_tag : unique_tags) {
        std::cout << unique_tag << " ";
    }
    std::cout << std::endl;
    for (auto row_tag : unique_tags) {
        auto row_iter = confused_matrix.find(row_tag);
        std::unordered_map<std::string, int> row_count_map;
        if (row_iter != confused_matrix.end()) {
            row_count_map = row_iter->second;
        }

        std::cout << row_tag << " ";
        for (auto col_tag : unique_tags) {
            double p = 0.0;
            auto total_iter = row_count_map.find("total_count");
            if (total_iter != row_count_map.end()) {
                auto col_iter = row_count_map.find(col_tag);
                if (col_iter != row_count_map.end()) {
                    p = (double) col_iter->second / total_iter->second;
                }
            }
            if (p == 0.0)
                std::cout << " " << " ";
            else
                std::cout << p << " ";
        }
        std::cout << std::endl;
    }
}


//
// 尤度をlogで取った値を返す関数
// @param vocab_size: 学習時の総単語数
// @param obs_word_count_map: タグから単語へのmap
// @param word: 今回尤度を計算する単語
//
double calcLogObservationLikelihood(const int& vocab_size,
                                 const std::unordered_map<std::string, int>& obs_word_count_map,
                                 const std::string& word)
{
    auto owcm_iter = obs_word_count_map.find(word);
    int obs_word_count = 0;
    int obs_tag_total_count = 0;
    if (owcm_iter != obs_word_count_map.end())
        obs_word_count = owcm_iter->second;
    owcm_iter = obs_word_count_map.find("total_count");
    if (owcm_iter != obs_word_count_map.end())
        obs_tag_total_count = owcm_iter->second;
    // 分子分母
    int numer = obs_word_count + 1;
    int denom = obs_tag_total_count + vocab_size + 1;
    double obs_logp = log((double) numer / denom);
    return obs_logp;
}
