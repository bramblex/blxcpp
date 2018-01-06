// FunctionTraits.hpp
#ifndef BLXCPP_FUNCTIONTRAITS_HPP
#define BLXCPP_FUNCTIONTRAITS_HPP

#include <functional>

namespace blxcpp {

template<typename T>
struct function_traits;

template<typename Ret, typename ...Args>
struct function_traits<Ret(Args...)> {
    enum { size = sizeof...(Args) };
    typedef Ret function_type(Args...);
    typedef Ret return_type;
    using stl_function_type = std::function<function_type>;
    typedef Ret(*pointer)(Args...);

    template<size_t I>
    struct args {
        static_assert(I < size, "Index is out of range, index must less than sizeof Args");

        // std::tuple_element<i, std::tuple<Args...>type  可以获取第 i 个 Args
        using type = typename std::tuple_element<I, std::tuple<Args...>>::type;
    };
};

// 函数指针的特化版本
template <typename Ret, typename... Args>
struct function_traits<Ret(*)(Args...)>
    : function_traits<Ret(Args...)> { };

// stl function 的特化版本
template <typename Ret, typename... Args>
struct function_traits<std::function<Ret(Args...)>>
    : function_traits<Ret(Args...)> { };

// 成员函数特化版本
#define FUNCTION_TRAITS(...) \
template <typename ReturnType, typename ClassType, typename... Args> \
struct function_traits<ReturnType(ClassType::*)(Args...) __VA_ARGS__> \
    : function_traits<ReturnType(Args...)> { }

// const / volatile 特化版本
FUNCTION_TRAITS();
FUNCTION_TRAITS(const);
FUNCTION_TRAITS(volatile);
FUNCTION_TRAITS(const volatile);

#undef FUNCTION_TRAITS

// 函数对象特化
template<typename Callable>
struct function_traits
    : function_traits<decltype (&Callable::operator())> { };

}

#endif // BLXCPP_FUNCTIONTRAITS_HPP
