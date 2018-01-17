// Timer.cpp
#include "Timer.hpp"

namespace blxcpp {

Timer::Time Timer::now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now().time_since_epoch()).count();
}

Timer::ID Timer::nextId() {
    if (m_next_id_ < 0) m_next_id_ = 0;
    return m_next_id_++;
}

Timer::Timer(Timer::Time current)
    : m_current(current) { }

Timer::Timer()
    : m_current(now()) { }

Timer::Ref Timer::setTimeout(Timer::Time timeout, bool repeat, const std::function<void ()> &func){
    std::lock_guard<std::mutex> sp(m_lock);
    ID id = nextId();
    m_containers[id] = Timeout(id, m_current, repeat, timeout, func);
    m_queue.push(&m_containers[id]);
    return Ref(this, id);
}

void Timer::tick(Timer::Time time){
    std::lock_guard<std::mutex> sp(m_lock);
    m_current = time;

    if (!m_containers.empty()) {

        std::vector<Timeout*> removed;
        std::vector<Timeout*> next;

        while (!m_queue.empty()) {
            Timeout* t = m_queue.top();

            // 如果是
            if (!t->m_is_activated) {
                m_queue.pop();
                removed.emplace_back(t);
                continue;
            }

            if (t->m_expried_at > m_current) break;
            m_queue.pop();

            if (t->m_is_repeated) {
                t->m_expried_at = m_current + t->m_timeout;
                next.emplace_back(t);
            } else {
                removed.emplace_back(t);
            }

            t->m_func();
        }

        for (Timeout* t : removed){
            m_containers.erase(t->m_id);
        }

        for (Timeout* t : next) {
            m_queue.push(t);
        }

    }
}

void Timer::clear(Timer::ID id) {
    if (m_containers.find(id) != m_containers.end()) {
        std::lock_guard<std::mutex> sp(m_lock);
        m_containers[id].m_is_activated = false;
    }
}

bool Timer::empty() {
    return m_containers.empty();
}

Timer::Ref::Ref(Timer *timer, Timer::ID id)
    :  m_timer(timer), m_id(id) { }

void Timer::Ref::clear(){ m_timer->clear(m_id); }

Timer::Timeout::Timeout() { }

Timer::Timeout::Timeout(Timer::ID id, Timer::Time now, bool is_repeated, Timer::Time timeout, const std::function<void ()> &func)
    : m_id(id)
    , m_is_activated(true)
    , m_is_repeated(is_repeated)
    , m_timeout(timeout)
    , m_expried_at(now + m_timeout)
    , m_func(func) { }

bool Timer::Timeout::PtrLess::operator()(const Timer::Timeout *l, const Timer::Timeout *r){
    return (l->m_expried_at > r->m_expried_at);
}


}
