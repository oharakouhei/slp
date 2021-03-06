//
// viterbiを計算するスクリプト
//
////////////////////// 実行例 ////////////////////////////
// $ cat test.txt | ./tagger model
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
// total tags: 47377
// known word tags: 44075, correct known word tags: 39780
// known_word_precision: 0.902552
// unknown word tags: 3302, correct unknown word tags: 944
// unknown_word_precision: 0.285887
//
#include "common.h"

class Tagger {
public:
    std::vector<std::string> id2tag, id2word;
    std::vector<std::vector<double>> transition_prob; // tag (pre) -> tag
    std::vector<double> start_transition_prob;
    std::vector<double> end_transition_prob;
    std::vector<std::vector<double>> emission_prob; // tag -> word
    Tagger() {
        transition_prob = {}, start_transition_prob = {},
        end_transition_prob = {}, emission_prob = {};
    }
    // debug
    void show_all () {
        std::cout << "{" << std::endl
        << "  id2tag: " << id2tag << std::endl
        << "  id2word: " << id2word << std::endl
        << "  transition: " << transition_prob << std::endl
        << "  start_transition: " << start_transition_prob << std::endl
        << "  end_transition: " << end_transition_prob << std::endl
        << "  emission: " << emission_prob << std::endl
        << "}" << std::endl;
    }
};

void read_model(char* model_file, Tagger* tagger) {
    char line[1 << 21];
    FILE* fp = fopen(model_file, "r");
//    std::unordered_map<std::string, int> word2id, tag2id;
//    int word_id(-1), tag_id(-1), prev_tag_id(-1);
//
    int model_flg = 0;
    while (fgets(line, 1 << 21, fp)) {
        if (line[0] == '\n') {
            ++model_flg;
            continue;
        }
        char* mv = line;
        char* stay = mv;
        size_t len = strlen(mv);
        switch (model_flg) {
            case 0: {
                // id2tag
                std::string tag = std::string(stay, len);
                tagger->id2tag.push_back(tag);
                break;
            }
            case 1: {
                // id2word
                std::string word = std::string(stay, len);
                tagger->id2word.push_back(word);
                break;
            }
            case 2:
                // start_transition prob
                for (; *mv != '\n'; ++mv)
                    tagger->start_transition_prob.push_back(log(std::strtod(mv, &mv)));
                break;
            case 3:
                // end_transition prob
                for (; *mv != '\n'; ++mv)
                    tagger->end_transition_prob.push_back(log(std::strtod(mv, &mv)));
                break;
            case 4: {
                // transition prob
                std::vector<double> pretag2tag_prob;
                for (; *mv != '\n'; ++mv)
                    pretag2tag_prob.push_back(log(std::strtod(mv, &mv)));
                tagger->transition_prob.push_back(pretag2tag_prob);
                break;
            }
            case 5: {
                // emission prob
                std::vector<double> tag2word_prob;
                for (; *mv != '\n'; ++mv)
                    tag2word_prob.push_back(log(std::strtod(mv, &mv)));
                tagger->emission_prob.push_back(tag2word_prob);
                break;
            }
            default:
                // if there is something left
                std::cerr << mv << std::endl;
        }
    }
    // tagger->show_all();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage example: zcat test.txt.gz | " << argv[0] << " model" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    Tagger tagger;

    read_model(argv[1], &tagger);

    return 0;
}