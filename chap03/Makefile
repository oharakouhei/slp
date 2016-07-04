main_h_files  :=  ngram.h utility.h
main_cpp_files := main.cpp ngram.cpp utility.cpp
cdr2sentence_cpp_files := cdr2sentence.cpp
opts :=
opts += -O3
opts += -std=c++11
debug := -g

test_opts := -lpthread
test_opts += -Wall

CXXFLAGS := $(opts)

.PHONY: all
all : main cdr2sentence

.PHONY: main
main : $(main_cpp_files) $(main_h_files)
	g++ -o $@ $(CXXFLAGS) $(main_cpp_files)

.PHONY: cdr2sentence
cdr2sentence : $(cdr2sentence_cpp_files)
	g++ -o $@ $(CXXFLAGS) $(cdr2sentence_cpp_files)

.PHONY: run
run:
	./main
	./cdr2sentence

debug: $(main_cpp_files) $(main_h_files)
	g++ $(debug) -o $@ $(CXXFLAGS) $(main_cpp_files)
	gdb ./$@


.PHONY: clean
clean :
	rm -f main
	rm -f cdr2sentence
	rm -f debug
	rm -rf *.dSYM
