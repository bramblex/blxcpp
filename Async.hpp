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


namespace blxcpp {

class AsyncEventLoop {
private:

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

    Timer::Ref setTimeout(Timer::Time t, const std::function<void()> func) {
        auto event_loop = this;
        return m_timer.setTimeout(t, false, [event_loop, func](){
            std::lock_guard<std::mutex> sp(event_loop->m_queue_lock);
            event_loop->pushEvent(Event(func));
        });
    }

    Timer::Ref setInterval(Timer::Time t, const std::function<void()>& func) {
        auto event_loop = this;
        return m_timer.setTimeout(t, true, [event_loop, func](){
        std::lock_guard<std::mutex> sp(event_loop->m_queue_lock);
            event_loop->pushEvent(Event(func));
        });
    }

};

//template <typename T=void>
//class AsyncEventLoop {
//private:

//    std::mutex m_queue_lock;
//    std::deque<std::function<void()>> m_queue;

//public:


//private:
//    static AsyncEventLoop<T>* m_instance;
//    static std::sig_atomic_t m_signal;

//public:

//    static AsyncEventLoop<T>* get() {
//        if (m_instance == nullptr) {
//            m_instance = new AsyncEventLoop();
//            std::signal(SIGINT, [](int signal){
//                AsyncEventLoop<T>::m_signal = signal;
//            });
//        }
//        return m_instance;
//    }

//    static void run() {
//        AsyncEventLoop<T>* event_loop = get();
//        while(m_signal == 0) {

//            if (event_loop->m_queue.size() > 0)
//                continue;

//            std::function<void()> func;
//            {
//                std::lock_guard<std::mutex> sp(event_loop->m_queue_lock);
//                func = event_loop->m_queue.front();
//                event_loop->m_queue.pop_front();
//            }

//            func();
//            std::this_thread::sleep_for(std::chrono::milliseconds(6));

//        }
//    }
//};

//template <typename T=void>
//class AsyncLock {
//private:
//    static AsyncLock<T>* m_instance;

//public:

//    std::atomic<int> m_count;
//    std::mutex m_lock;

//    static AsyncLock<T>* get(){
//        if (m_instance == nullptr) {
//            m_instance = new AsyncLock<T>();
//        }
//        return m_instance;
//    }

//};

//template <typename T>
//AsyncLock<T>* AsyncLock<T>::m_instance = nullptr;

//template<typename Func>
//class Async {
//private:
//    const Func m_func;

//public:
//    Async(const Func& func)
//        : m_func(func) { }

//    template<typename ...Args>
//    void operator()(Args... args) {
//        return std::thread(m_func, std::forward<Args>(args)...).detach();
//    }

//    template<typename ...Args>
//    void sync(Args... args) {
//        m_func(std::forward<Args>(args)...);
//    }

//};

//template<typename Func>
//Async<Func> async(const Func& func) {
//    return Async<Func>(func);
//}

}

#endif // BLXCPP_ASYNC_HPP
