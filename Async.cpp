// Async.cpp
#include "Async.hpp"

namespace blxcpp {

// init
AsyncEventLoop* AsyncEventLoop::global = nullptr;
std::sig_atomic_t AsyncEventLoop::singal = 0;

void eventLoop() {
    std::signal(SIGINT, [](int i){
        AsyncEventLoop::singal = i;
    });
    AsyncEventLoop::getGlobal()->epoll(6, [](){
        return AsyncEventLoop::singal > 0;
    });
}

Timer::Ref setInterval(Timer::Time t, const std::function<void ()> &func) {
    return AsyncEventLoop::getGlobal()->setInterval(t, func);
}

Timer::Ref setTimeout(Timer::Time t, const std::function<void ()> &func) {
    return AsyncEventLoop::getGlobal()->setTimeout(t, func);
}

void AsyncEventLoop::epoll() {
    return epoll(6, [](){ return false; });
}

void AsyncEventLoop::epoll(int64_t interval, const std::function<bool ()> &stop){
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

void AsyncEventLoop::pushEvent(const AsyncEventLoop::Event &event) {
    std::lock_guard<std::mutex> sp(m_queue_lock);
    m_queue.push_back(event);
}

void AsyncEventLoop::pushNextTick(const AsyncEventLoop::Event &event) {
    std::lock_guard<std::mutex> sp(m_queue_lock);
    m_queue.push_front(event);
}

Timer::Ref AsyncEventLoop::setTimeout(Timer::Time t, const std::function<void ()> &func) {
    auto event_loop = this;
    return m_timer.setTimeout(t, false, [event_loop, func](){
        event_loop->m_queue.push_back(Event(func));
    });
}

Timer::Ref AsyncEventLoop::setInterval(Timer::Time t, const std::function<void ()> &func) {
    auto event_loop = this;
    return m_timer.setTimeout(t, true, [event_loop, func](){
        event_loop->m_queue.push_back(Event(func));
    });
}

AsyncEventLoop *AsyncEventLoop::getGlobal(){
    if (global == nullptr) {
        global = new AsyncEventLoop();
    }
    return global;
}

}
