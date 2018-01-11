// Async.hpp
#ifndef BLXCPP_ASYNC_HPP
#define BLXCPP_ASYNC_HPP

#include "function_traits.hpp"
#include "Timer.hpp"
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
        inline Event() { }
        inline Event(const std::function<void()>& func)
            : m_func(func) { }

        inline void operator()() { m_func(); }
    };

    template <typename Func>
    class Async;

    template <typename Ret, typename ...Args>
    class Async<Ret(Args...)> {
    private:

        AsyncEventLoop* m_event_loop;
        std::function<Ret(Args...)> m_func;

    public:

        using OnSuccess = std::function<void(Ret)>;
        using OnFailed = std::function<void(std::exception)>;

        Async(AsyncEventLoop* event_loop, const std::function<Ret(Args...)>& func)
            : m_event_loop(event_loop), m_func(func) { }

        void operator()(Args... args, const OnSuccess& onSuccess, const OnFailed& onFailed) {
            auto func = m_func;
            AsyncEventLoop* event_loop = m_event_loop;
            std::thread([&args..., event_loop, func, onSuccess, onFailed](){
                event_loop->m_thread_count++;

                Event event;
                try {
                    event = std::function<void()>(std::bind(onSuccess, func(std::forward<Args>(args)...)));
                } catch (std::exception exp) {
                    event = std::function<void()>(std::bind(onFailed, exp));
                }

                event_loop->pushEvent(event);

                event_loop->m_thread_count--;
            }).detach();
        }

        void operator()(Args... args) {
            return operator()(args... ,  [](Ret){}, [](std::exception){});
        }

        void operator()(Args... args, const OnSuccess& onSuccess){
            return operator()(args..., onSuccess, [](std::exception exp){ throw exp; });
        }

        Ret sync(Args... args) {
            return m_func(std::forward<Args>(args)...);
        }


    };


    template <typename ...Args>
    class Async<void(Args...)> {
    private:

        AsyncEventLoop* m_event_loop;
        std::function<void(Args...)> m_func;

    public:

        using OnSuccess = std::function<void()>;
        using OnFailed = std::function<void(std::exception)>;

        Async(AsyncEventLoop* event_loop, const std::function<void(Args...)>& func)
            : m_event_loop(event_loop), m_func(func) { }

        void operator()(Args... args, const OnSuccess& onSuccess, const OnFailed& onFailed) const {
            auto func = m_func;
            AsyncEventLoop* event_loop = m_event_loop;
            std::thread([&args..., event_loop, func, onSuccess, onFailed](){
                event_loop->m_thread_count++;

                Event event;
                try {
                    event = onSuccess;
                } catch (std::exception exp) {
                    event = std::function<void()>(std::bind(onFailed, exp));
                }

                event_loop->pushEvent(event);

                event_loop->m_thread_count--;
            }).detach();
        }

        void operator()(Args... args) const {
            return operator()(args... ,  [](){}, [](std::exception){});
        }

        void operator()(Args... args, const OnSuccess& onSuccess) const {
            return operator()(args..., onSuccess, [](std::exception exp){ throw exp; });
        }

        void sync(Args... args) const {
            return m_func(std::forward<Args>(args)...);
        }
    };

private:
    std::mutex m_queue_lock;
    std::deque<Event> m_queue;
    std::atomic<int> m_thread_count;
    Timer m_timer;

public:

    AsyncEventLoop()
        : m_thread_count(0) { }

    template<typename Func>
    Async<typename function_traits<Func>::function_type> async(const Func& func) {
        return Async<typename function_traits<Func>::function_type>(this, func);
    }

    void epoll() {
        return epoll(6, [](){ return false; });
    }

    void epoll(int64_t interval, const std::function<bool()>& stop){
        while (!stop() && (m_thread_count > 0 || !m_queue.empty() || !m_timer.empty())) {

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

    void pushEvent(const Event& event) {
        std::lock_guard<std::mutex> sp(m_queue_lock);
        m_queue.push_back(event);
    }

    void pushNextTick(const Event& event) {
        std::lock_guard<std::mutex> sp(m_queue_lock);
        m_queue.push_front(event);
    }

    Timer::Ref setTimeout(Timer::Time t, const std::function<void()>& func) {
        auto event_loop = this;
        return m_timer.setTimeout(t, false, [event_loop, func](){
            event_loop->m_queue.push_back(Event(func));
        });
    }

    Timer::Ref setInterval(Timer::Time t, const std::function<void()>& func) {
        auto event_loop = this;
        return m_timer.setTimeout(t, true, [event_loop, func](){
            event_loop->m_queue.push_back(Event(func));
        });
    }

public:
    static AsyncEventLoop* const global;
    static std::sig_atomic_t singal;
};

AsyncEventLoop* const AsyncEventLoop::global = new AsyncEventLoop();
std::sig_atomic_t AsyncEventLoop::singal = 0;


template<typename Func>
auto async(const Func& func) -> decltype (AsyncEventLoop::global->async(func)) {
    return AsyncEventLoop::global->async(func);
}

Timer::Ref setTimeout(Timer::Time t, const std::function<void()>& func) {
    return AsyncEventLoop::global->setTimeout(t, func);
}

Timer::Ref setInterval(Timer::Time t, const std::function<void()>& func) {
    return AsyncEventLoop::global->setInterval(t, func);
}

void eventLoop() {
    std::signal(SIGINT, [](int i){
        AsyncEventLoop::singal = i;
    });
    AsyncEventLoop::global->epoll(6, [](){
        return AsyncEventLoop::singal > 0;
    });
}

}

#endif // BLXCPP_ASYNC_HPP
