// Async.hpp
#ifndef BLXCPP_ASYNC_HPP
#define BLXCPP_ASYNC_HPP

#include "function_traits.hpp"
#include "Timer.hpp"
#include "ThreadPool.hpp"

#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <csignal>
#include <chrono>
#include <map>
#include <iostream>


namespace blxcpp {

class AsyncEventLoop {
public:
    class Event {
    private:
        std::function<void()> m_func;

    public:
        Event() { }
        Event(const std::function<void()>& func)
            : m_func(func) { }

        void operator()() { m_func(); }
    };

    template <typename Func>
    class Async;

    template <typename Arg, typename Ret>
    class Continuation;

private:
    std::mutex m_queue_lock;
    std::deque<Event> m_queue;
    ThreadPool m_thread_pool;
    Timer m_timer;

public:

    template<typename Func>
    inline Async<typename function_traits<Func>::function_type> async(const Func& func) {
        return Async<typename function_traits<Func>::function_type>(this, func);
    }

     void epoll();
     void epoll(int64_t interval, const std::function<bool()>& stop);
     void pushEvent(const Event& event);

    void pushNextTick(const Event& event);
    Timer::Ref setTimeout(Timer::Time t, const std::function<void()>& func);
    Timer::Ref setInterval(Timer::Time t, const std::function<void()>& func);

private:
    static AsyncEventLoop* global;

public:
    static std::sig_atomic_t singal;
    static AsyncEventLoop *getGlobal();
};

template <typename Ret, typename ...Args>
class AsyncEventLoop::Async<Ret(Args...)> {
public:

    template<typename, typename Arg>
    struct MakeCallback {
        using type = void(Arg);
    };

    template<typename IGNORE>
    struct MakeCallback<IGNORE, void> {
        using type = void();
    };

    using Callback = std::function<typename MakeCallback<void, Ret>::type>;
    using Func = std::function<Ret(Args...)>;

private:

    AsyncEventLoop* m_event_loop;
    std::function<Ret(Args...)> m_func;

    template<typename, bool EmptyReturn>
    struct Runer;

    template<typename IGNORE>
    struct Runer<IGNORE, true> {
        static void run(AsyncEventLoop* event_loop, const Func& func, const Callback& callback, Args... args){
            event_loop->m_thread_pool.put(std::bind([event_loop, func, callback](Args... args){
                func(std::forward<Args>(args)...);
                Event event = callback;
                event_loop->pushEvent(event);
            }, std::forward<Args>(args)...));
        }
    };

    template<typename IGNORE>
    struct Runer<IGNORE, false> {
        static void run(AsyncEventLoop* event_loop, const Func& func, const Callback& callback, Args... args){
            event_loop->m_thread_pool.put(std::bind([event_loop, func, callback](Args... args){
                Event event = std::function<void()>(std::bind(callback, func(std::forward<Args>(args)...)));
                event_loop->pushEvent(event);
            }, std::forward<Args>(args)...));
        }
    };


public:

    Async(AsyncEventLoop* event_loop, const std::function<Ret(Args...)>& func)
        : m_event_loop(event_loop), m_func(func) { }

    void operator()(Args... args, const Callback& callback) {
        auto func = m_func;
        AsyncEventLoop* event_loop = m_event_loop;
        Runer<void, std::is_same<Ret, void>::value>::run(event_loop, func, callback, std::forward<Args>(args)...);
    }

    void operator()(Args... args) {
        m_event_loop->m_thread_pool.put(std::bind(m_func, std::forward<Args>(args)...));
    }

    Ret sync(Args... args) {
        return m_func(std::forward<Args>(args)...);
    }

};


template<typename Func>
auto async(const Func& func) -> decltype (AsyncEventLoop::getGlobal()->async(func)) {
    return AsyncEventLoop::getGlobal()->async(func);
}

Timer::Ref setTimeout(Timer::Time t, const std::function<void()>& func);
Timer::Ref setInterval(Timer::Time t, const std::function<void()>& func);
void eventLoop();


}

#endif // BLXCPP_ASYNC_HPP
