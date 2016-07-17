train2tagseq_h_files := common.h
train2tagseq_cpp_files := train2tagseq.cpp common.cpp
opts :=
opts += -O3
opts += -std=c++11
debug := -g

test_opts := -lpthread
test_opts += -Wall

CXXFLAGS := $(opts)

.PHONY: all
all : train2tagseq

.PHONY: train2tagseq
train2tagseq : $(train2tagseq_cpp_files) $(train2tagseq_h_files)
	g++ -o $@ $(CXXFLAGS) $(train2tagseq_cpp_files)

.PHONY: clean
clean :
	rm -f train2tagseq
	rm -f debug
	rm -rf *.dSYM
