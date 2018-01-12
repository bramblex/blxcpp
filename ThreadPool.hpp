// ThreadPool.hpp
#ifndef BLXCPP_THREADPOOL_HPP
#define BLXCPP_THREADPOOL_HPP

#include <thread>
#include <functional>
#include <set>

namespace blxcpp {

template<size_t MIN, size_t MAX>
class ThreadPool {
public:

    constexpr static size_t min = MIN;
    constexpr static size_t max = MAX;

    class Thread {
    };

private:
    std::set<Thread*> m_busy; // 忙碌线程
    std::set<Thread*> m_free; // 空闲线程
    std::set<Thread*> m_excl; // 独占线程

public:

    ThreadPool() {
    }

};

}

#endif // BLXCPP_THREADPOOL_HPP
