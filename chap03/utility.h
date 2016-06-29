//
// Created by OharaKohei on 2016/06/29.
//

#ifndef CHAP03_UTILITY_H
#define CHAP03_UTILITY_H

#include <iostream>
#include <vector>

//
// 演算子 << のオーバーロード
// vectorの中身を出力する
// 基本出力: [1, 2, 4,...]
// vectorの中身がvectorであっても以下のように出力する
// [[2, 3,...],[3, 4,...],[2, 1,...],...]
// テンプレートなのでヘッダに定義を書いている
//
template<typename T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
    out << "[";
    size_t last = v.size() - 1;
    for (size_t i = 0; i < v.size(); ++i) {
        out << v[i];
        if (i != last)
            out << ", ";
    }
    out << "]";
    return out;
}

#endif //CHAP03_UTILITY_H
