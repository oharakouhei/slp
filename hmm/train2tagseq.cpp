//
// train.txt（形式は以下）からタグの並びを抽出するスクリプト
//
// 入力 (train.txt)
// He PRP B-NP
// is VBZ B-VP
// (He is から始まる一文の他の単語のタグ等の情報)
// . . O
// (改行で文の区切り)
//
// 出力 (一文のタグの並びを一列に)
// PRP VBZ ... .
//
//
#include "common.h"

int main() {
    char char_line[1 << 21];
    while (fgets(char_line, 1 << 21, stdin))
    {
        if (char_line[0] == '\n')
        {
            std::cout << std::endl;
        }
        else
        {
            std::string line = char_line;
            size_t first_delim_pos = line.find(TRAIN_DELIMITER);
            size_t second_delim_pos = line.find(TRAIN_DELIMITER, first_delim_pos+1);
            int tag_len = second_delim_pos-first_delim_pos-1;
            std::string tag = line.substr(first_delim_pos+1, tag_len);
            std::cout << tag << SENTENCE_DELIMITER;
        }
    }
    return 0;
}