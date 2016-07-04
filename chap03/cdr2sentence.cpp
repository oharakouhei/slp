//
// 形態素解析結果のcdrファイルを、文に変換するスクリプト
//
#include <iostream>
#include <fstream>

std::string SENTENCE_END_SYMBOL = "EOS";

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " cdr_file" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string cdr_file = argv[1];
    std::ifstream input(cdr_file);
    if (!input) {
        std::cerr << "cannot find cdr file." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string line;
    std::string delimiter = "\t";
    while (std::getline(input, line))
    {
        std::string surface = line.substr(0, line.find(delimiter));
        if (surface == SENTENCE_END_SYMBOL)
        {
            std::cout << std::endl;
        }
        else
        {
            std::cout << surface << " ";
        }
    }
}