// Timer.hpp
#ifndef BLXCPP_TIMER_HPP
#define BLXCPP_TIMER_HPP


#include <chrono>
#include <functional>
#include <mutex>
#include <map>
#include <set>

namespace blxcpp {

class Timer {
public:

    using ID = int64_t;
    using Time = int64_t;

    inline static Time now () {
        return std::chrono::duration_cast<std::chrono::milliseconds>
                (std::chrono::system_clock::now().time_since_epoch()).count();
    }

    class Ref {
    private:
        Timer* m_timer;
        ID m_id;
    public:
        inline Ref(Timer* timer, ID id)
            :  m_timer(timer), m_id(id) { }

        inline void clear(){ m_timer->clear(m_id); }
    };

private:

    class Timeout {
    public:
        ID m_id;
        bool m_is_repeated; // 是否重复
        Time m_timeout; // 设置的 timeout 时间
        Time m_expried_at; // 超时时间
        std::function<void()> m_func; // 存储的函数

        inline Timeout() { }
        inline Timeout(ID id, Time now, bool is_repeated, Time timeout, const std::function<void()>& func)
            : m_id(id)
            , m_is_repeated(is_repeated)
            , m_timeout(timeout)
            , m_expried_at(now + m_timeout)
            , m_func(func) { }


        struct PtrLess {
            inline bool operator()(const Timeout* l, const Timeout* r){
                return l->m_expried_at < r->m_expried_at;
            }
        };
    };

private:
    std::mutex m_lock;
    std::map<ID, Timeout> m_containers;
    std::set<Timeout*, Timeout::PtrLess> m_queue;

    ID m_next_id_ = 0;
    inline ID nextId() {
        if (m_next_id_ < 0) m_next_id_ = 0;
        return m_next_id_++;
    }

    Time m_current;

public:

    inline Timer(Time current)
        : m_current(current) { }

    inline Timer()
        : m_current(now()) { }

    inline Ref setTimeout(Time timeout, bool repeat, const std::function<void()>& func){
        std::lock_guard<std::mutex> sp(m_lock);
        ID id = nextId();
        m_containers[id] = Timeout(id, m_current, repeat, timeout, func);
        m_queue.insert(&m_containers[id]);
        return Ref(this, id);
    }

    inline void tick(Time time){
        std::lock_guard<std::mutex> sp(m_lock);
        m_current = time;

        if (!m_containers.empty()) {

            std::set<Timeout*> removed;
            std::set<Timeout*> next;

            for (auto& t : m_queue) {
                if (t->m_expried_at > m_current) break;

                if (t->m_is_repeated) {
                    t->m_expried_at = m_current + t->m_timeout;
                    next.insert(t);
                } else {
                    removed.insert(t);
                }

                t->m_func();
            }

            for (Timeout* t : removed)
                m_containers.erase(t->m_id);

            for (Timeout* t : next)
                m_queue.insert(t);

        }
    }

    inline void clear(ID id) {
        if (m_containers.find(id) != m_containers.end()) {
            std::lock_guard<std::mutex> sp(m_lock);
            auto p = &m_containers[id];
            m_queue.erase(p);
            m_containers.erase(id);
        }
    }

    inline bool empty() {
        return m_containers.empty();
    }

};

}


#endif // BLXCPP_TIMER_HPP
