// Lazy.hpp
#ifndef BLXCPP_LAZY_HPP
#define BLXCPP_LAZY_HPP

#include <functional>
#include <type_traits>
#include <utility>
#include <cstdlib>
#include <stdexcept>

namespace blxcpp {

template <typename R>
class Lazy {
private:

    // 测量 Result Buffer
    using ResultBuffer = typename std::aligned_storage<sizeof (R), alignof (R)>::type;

    // 强制转换指针用帮助模板
    template<typename Target, typename Origin>
    static Target* pointer_cast(Origin pointer) {
        return (reinterpret_cast<Target*>(reinterpret_cast<size_t>(pointer)));
    }

    using Func = std::function<R()>;
    const Func m_func;
    mutable bool m_has_thunked = false; // 是否 thunk
    mutable ResultBuffer m_result; // 在 const 下也可以获得值

    void thunk() const {
        new (&m_result) R(m_func());
        m_has_thunked = true;
    }

public:

    Lazy(Func const& func)
        : m_func(func) { }

    // 右值版本，用来减少构造函数测试
    Lazy(Func const&& func)
        : m_func(std::move(func)) { }

    R const& operator()() const {
        if (!m_has_thunked) thunk(); // 若是没有计算则执行 thunk
        return *(pointer_cast<R>(&m_result));
    }
};

template <typename F>
auto lazy(F const& func) -> Lazy<decltype(func())> {
    return Lazy<decltype(func())>(func);
}

}

#endif // BLXCPP_LAZY_HPP
