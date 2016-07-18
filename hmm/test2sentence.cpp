//
// test.txt（形式は以下）を一行一文に変換するスクリプト
// (中身はほとんどtrain2tagseq.cppと同じ)
//
////////////////////// 実行例 ////////////////////////////
// $ cat test.txt | ./test2sentence | head -2
// Rockwell International Corp. 's Tulsa ... provide structural parts for Boeing 's 747 jetliners .
// Rockwell said the agreement calls for it to supply 200 additional so-called shipsets for the planes .
//
//////////////////// 入力 (train.txt) /////////////////////
// He PRP B-NP
// is VBZ B-VP
// (He is から始まる一文の他の単語のタグ等の情報)
// deficits NNS I-NP
// . . O
// (改行で文の区切り)
//
//////////////// 出力例 (一文のタグの並びを一列に) ///////////////
// He is ... deficits .
// ...
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
            std::string word = line.substr(0, line.find(SENTENCE_DELIMITER));
            std::cout << word << SENTENCE_DELIMITER;
        }
    }
    return 0;
}