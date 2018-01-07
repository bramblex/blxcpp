// Guard.hpp
#ifndef BLXCPP_GUARD_HPP
#define BLXCPP_GUARD_HPP

#include "function_traits.hpp"

namespace blxcpp {

template<typename T>
class Guard {
private:
    mutable bool m_is_dismissed = false; // 标记为 mutable
    const std::function<T()> m_func;

public:
    Guard(const std::function<T()>& func)
        : m_func(func) { }

    Guard(const std::function<T()>&& func)
        : m_func(std::move(func)) { }

    ~Guard() { if (!m_is_dismissed) m_func(); }

    void dismiss () const { m_is_dismissed = true; }
};

template<typename Func>
auto guard(Func func) -> Guard<decltype (func())> {
    return Guard<decltype (func())>(std::forward<Func>(func));
}

}

#endif // BLXCPP_GUARD_HPP
