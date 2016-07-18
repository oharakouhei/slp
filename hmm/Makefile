train2tagseq_h_files := common.h
train2tagseq_cpp_files := train2tagseq.cpp common.cpp
tagseq2tagseq_count_h_files := common.h
tagseq2tagseq_count_cpp_files := tagseq2tagseq_count.cpp common.cpp
train2tagword_count_h_files := common.h
train2tagword_count_cpp_files := train2tagword_count.cpp common.cpp
test2sentence_h_files := common.h
test2sentence_cpp_files := test2sentence.cpp common.cpp
viterbi_h_files := common.h
viterbi_cpp_files := viterbi.cpp common.cpp
opts :=
opts += -O3
opts += -std=c++11
debug := -g

test_opts := -lpthread
test_opts += -Wall

CXXFLAGS := $(opts)

.PHONY: all
all : train2tagseq tagseq2tagseq_count train2tagword_count test2sentence viterbi

.PHONY: train2tagseq
train2tagseq : $(train2tagseq_cpp_files) $(train2tagseq_h_files)
	g++ -o $@ $(CXXFLAGS) $(train2tagseq_cpp_files)

.PHONY: tagseq2tagseq_count
tagseq2tagseq_count : $(tagseq2tagseq_count_cpp_files) $(tagseq2tagseq_count_h_files)
	g++ -o $@ $(CXXFLAGS) $(tagseq2tagseq_count_cpp_files)

.PHONY: train2tagword_count
train2tagword_count : $(train2tagword_count_cpp_files) $(train2tagword_count_h_files)
	g++ -o $@ $(CXXFLAGS) $(train2tagword_count_cpp_files)

.PHONY: test2sentence
test2sentence : $(test2sentence_cpp_files) $(test2sentence_h_files)
	g++ -o $@ $(CXXFLAGS) $(test2sentence_cpp_files)

.PHONY: viterbi
viterbi : $(viterbi_cpp_files) $(viterbi_h_files)
	g++ -o $@ $(CXXFLAGS) $(viterbi_cpp_files)

.PHONY: clean
clean :
	rm -f train2tagseq
	rm -f tagseq2tagseq_count
	rm -f train2tagword_count
	rm -f test2sentence
	rm -f viterbi
	rm -rf *.dSYM
