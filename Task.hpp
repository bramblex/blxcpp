// Task.hpp
#ifndef BLXCPP_TASK_HPP
#define BLXCPP_TASK_HPP

#include "function_traits.hpp"

namespace blxcpp {

template<typename T>
class Task;

template <typename Ret, typename Arg>
class Task<Ret(Arg)> {
private:
    using Func = typename function_traits<Ret(Arg)>::stl_function_type;
    const Func m_func;

public:
    Task(const Func& func)
        : m_func(func) { }

    // 右值转移
    Task(const Func&& func)
        : m_func(std::move(func)) { }

    Ret run(Arg&& arg) {
        // forward 完美转发
        return m_func(std::forward<Arg>(arg));
    }

    template<typename Next>
    auto then(const Next& next)
            -> Task<typename function_traits<Next>::return_type(Arg)> {
        using NextRet = typename function_traits<Next>::return_type;
        auto func = std::move(m_func);
        return Task<NextRet(Arg)>([func, &next](Arg&& arg){
            return next(func(std::forward<Arg>(arg)));
        });
    }
};

template <typename Func>
auto task(Func const& func)
    -> Task<typename function_traits<Func>::function_type> {
    return Task<typename function_traits<Func>::function_type>(func);
}

}

#endif // BLXCPP_TASK_HPP
