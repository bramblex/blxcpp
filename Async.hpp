// Async.hpp
#ifndef BLXCPP_ASYNC_HPP
#define BLXCPP_ASYNC_HPP

#include "function_traits.hpp"
#include <thread>

namespace blxcpp {

template<typename Func>
class Async {
private:
    const Func m_func;

public:
    Async(const Func& func)
        : m_func(func) { }

    template<typename ...Args>
    void operator()(Args... args) {
        return std::thread(m_func, std::forward<Args>(args)...).detach();
    }

    template<typename ...Args>
    void sync(Args... args) {
        m_func(std::forward<Args>(args)...);
    }

};

template<typename Func>
Async<Func> async(const Func& func) {
    return Async<Func>(func);
}

}

#endif // BLXCPP_ASYNC_HPP
