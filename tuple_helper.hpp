// tuple_helper.hpp
#ifndef BLXCPP_TUPLE_HELPER_HPP
#define BLXCPP_TUPLE_HELPER_HPP

#include <tuple>
#include <iostream>

namespace blxcpp {

// @TODO:
// 这里以后再实现，就是拿 tuple 来存储多个元素

template <class Tuple, size_t N >
struct TuplePrinter {
    static void print(const Tuple& t) {
        TuplePrinter<Tuple, N - 1>::print(t);
        std::cout << ", " << std::get<N - 1>(t);
    }
};

template <class Tuple>
struct TuplePrinter<Tuple, 1> {
    static void print(const Tuple& t) {
        std::cout << std::get<0>(t);
    }
};

template<class... Args>
void printTuple(const std::tuple<Args...>& t) {
    std::cout << "(";
    // sizeof... (Args) 可以测量出 Args 的参数个数
    TuplePrinter<decltype(t), sizeof... (Args)>::print(t);
    std::cout << ")" << std::endl;
}

// 这样一波骚操作没看出来扎起干嘛
// 先放着
template<int...>
struct IndexTuple{ };

template<int N, int... Indexes>
struct MakeIndexes
    : MakeIndexes<N - 1, N - 1, Indexes...> { };

template<int... Indexes>
struct MakeIndexes<0, Indexes...> {
    using type = IndexTuple<Indexes...>;
};

//

}

#endif // BLXCPP_TUPLE_HELPER_HPP
