train2tagseq_h_files := common.h
train2tagseq_cpp_files := train2tagseq.cpp common.cpp
tagseq2tagseqcount_h_files := common.h
tagseq2tagseqcount_cpp_files := tagseq2tagseqcount.cpp common.cpp
opts :=
opts += -O3
opts += -std=c++11
debug := -g

test_opts := -lpthread
test_opts += -Wall

CXXFLAGS := $(opts)

.PHONY: all
all : train2tagseq tagseq2tagseqcount

.PHONY: train2tagseq
train2tagseq : $(train2tagseq_cpp_files) $(train2tagseq_h_files)
	g++ -o $@ $(CXXFLAGS) $(train2tagseq_cpp_files)

.PHONY: tagseq2tagseqcount
tagseq2tagseqcount : $(tagseq2tagseqcount_cpp_files) $(tagseq2tagseqcount_h_files)
	g++ -o $@ $(CXXFLAGS) $(tagseq2tagseqcount_cpp_files)

.PHONY: clean
clean :
	rm -f train2tagseq
	rm -f tagseq2tagseqcount
	rm -f debug
	rm -rf *.dSYM
