#include <iostream>
#include "ngram.h"

int main(int argc, char** argv)
{
//    if (argc < 3) {
//        std::cout << "Usage: " << argv[0] << " training_data_file test_data_file N(default 2)" << std::endl;
//    }
    std::string train_data_file = "/Users/kohei/Desktop/college/tkl/slp_meeting/NLP/n_gram/ishiwatari/data/mini2012.txt";
    std::string test_data_file = "/Users/kohei/Desktop/college/tkl/slp_meeting/NLP/n_gram/ishiwatari/data/mini2013.txt";
    int N = argc < 3 ? 2 : std::atoi(argv[3]);

    std::cout << N << std::endl;
    NGram ngram(N);
    ngram.train(train_data_file);
    return 0;
}
