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
private:

    template<typename, typename Arg>
    struct MakeCallback {
        using type = void(Arg);
    };

    template<typename IGNORE>
    struct MakeCallback<IGNORE, void> {
        using type = void();
    };

public:

    class Event {
    private:
        std::function<void()> m_func;

    public:
        inline Event() { }
        inline Event(const std::function<void()>& func)
            : m_func(func) { }

        inline void operator()() { m_func(); }
    };

    template <typename Func>
    class Async;

    template <typename Ret, typename ...Args>
    class Async<Ret(Args...)> {
    public:
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

    inline void epoll() {
        return epoll(6, [](){ return false; });
    }

    inline void epoll(int64_t interval, const std::function<bool()>& stop){
        while (!stop() && (m_thread_pool.busy() || !m_queue.empty() || !m_timer.empty())) {

            m_timer.tick(Timer::now());

            Event event;
            {
                std::lock_guard<std::mutex> sp(m_queue_lock);
                if (m_queue.size() == 0) continue;

                event = m_queue.front();
                m_queue.pop_front();
            }

            event();
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));

        }
    }

    inline void pushEvent(const Event& event) {
        std::lock_guard<std::mutex> sp(m_queue_lock);
        m_queue.push_back(event);
    }

    inline void pushNextTick(const Event& event) {
        std::lock_guard<std::mutex> sp(m_queue_lock);
        m_queue.push_front(event);
    }

    inline Timer::Ref setTimeout(Timer::Time t, const std::function<void()>& func) {
        auto event_loop = this;
        return m_timer.setTimeout(t, false, [event_loop, func](){
            event_loop->m_queue.push_back(Event(func));
        });
    }

    inline Timer::Ref setInterval(Timer::Time t, const std::function<void()>& func) {
        auto event_loop = this;
        return m_timer.setTimeout(t, true, [event_loop, func](){
            event_loop->m_queue.push_back(Event(func));
        });
    }

private:
    static AsyncEventLoop* global;

public:
    static std::sig_atomic_t singal;

    inline static AsyncEventLoop* getGlobal(){
        if (global == nullptr) {
            global = new AsyncEventLoop();
        }
        return global;
    }
};

// init
AsyncEventLoop* AsyncEventLoop::global = nullptr;
std::sig_atomic_t AsyncEventLoop::singal = 0;


template<typename Func>
inline auto async(const Func& func) -> decltype (AsyncEventLoop::getGlobal()->async(func)) {
    return AsyncEventLoop::getGlobal()->async(func);
}

inline Timer::Ref setTimeout(Timer::Time t, const std::function<void()>& func) {
    return AsyncEventLoop::getGlobal()->setTimeout(t, func);
}

inline Timer::Ref setInterval(Timer::Time t, const std::function<void()>& func) {
    return AsyncEventLoop::getGlobal()->setInterval(t, func);
}

inline void eventLoop() {
    std::signal(SIGINT, [](int i){
        AsyncEventLoop::singal = i;
    });
    AsyncEventLoop::getGlobal()->epoll(6, [](){
        return AsyncEventLoop::singal > 0;
    });
}


}

#endif // BLXCPP_ASYNC_HPP
