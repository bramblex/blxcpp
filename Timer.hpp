// Timer.hpp
#ifndef BLXCPP_TIMER_HPP
#define BLXCPP_TIMER_HPP


#include <chrono>
#include <functional>
#include <mutex>
#include <map>
#include <queue>
#include <vector>

namespace blxcpp {

class Timer {
public:

    using ID = int64_t;
    using Time = int64_t;

    static Time now ();

    class Ref {
    private:
        Timer* m_timer;
        ID m_id;
    public:
        Ref(Timer* timer, ID id);
        void clear();
    };

private:

    class Timeout {
    public:
        ID m_id;
        bool m_is_activated;
        bool m_is_repeated; // 是否重复
        Time m_timeout; // 设置的 timeout 时间
        Time m_expried_at; // 超时时间
        std::function<void()> m_func; // 存储的函数

        Timeout();
        Timeout(ID id, Time now, bool is_repeated, Time timeout, const std::function<void()>& func);
        struct PtrLess { bool operator()(const Timeout* l, const Timeout* r); };
    };

private:
    std::mutex m_lock;
    std::map<ID, Timeout> m_containers;
    std::priority_queue<Timeout*, std::vector<Timeout*>, Timeout::PtrLess> m_queue;

    ID m_next_id_ = 0;
    ID nextId();

    Time m_current;

public:

    Timer(Time current);
    Timer();
    Ref setTimeout(Time timeout, bool repeat, const std::function<void()>& func);
    void tick(Time time);
    void clear(ID id);
    bool empty();

};

}


#endif // BLXCPP_TIMER_HPP
