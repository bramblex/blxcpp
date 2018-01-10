// Async.hpp
#ifndef BLXCPP_ASYNC_HPP
#define BLXCPP_ASYNC_HPP

#include "function_traits.hpp"
#include <thread>
#include <atomic>
#include <mutex>


namespace blxcpp {

template <typename T=void>
class AsyncLock {
private:
    static AsyncLock<T>* m_instance;

public:

    std::atomic<int> m_count;
    std::mutex m_lock;

    static AsyncLock<T>* get(){
        if (m_instance == nullptr) {
            m_instance = new AsyncLock<T>();
        }
        return m_instance;
    }

};

template <typename T>
AsyncLock<T>* AsyncLock<T>::m_instance = nullptr;

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
