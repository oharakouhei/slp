cmake_minimum_required(VERSION 3.5)
project(chap03)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#set(SOURCE_FILES main.cpp ngram.cpp ngram.h utility.cpp utility.h)
#add_executable(chap03 ${SOURCE_FILES} )

add_executable(main main.cpp ngram.cpp ngram.h utility.cpp utility.h )
add_executable(cdr2sentence cdr2sentence.cpp common.h)
add_executable(sentence2ngram sentence2ngram.cpp common.h)