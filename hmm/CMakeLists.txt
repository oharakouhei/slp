cmake_minimum_required(VERSION 3.5)
project(hmm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(tran2tagseq train2tagseq.cpp common.cpp common.h)
add_executable(tagseq2tagseq_count tagseq2tagseq_count.cpp common.cpp common.h)
add_executable(train2tagword_count train2tagword_count.cpp common.cpp common.h)
