// ThreadPool.hpp
#ifndef BLXCPP_THREADPOOL_HPP
#define BLXCPP_THREADPOOL_HPP

#include <thread>
#include <functional>
#include <set>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <atomic>

#include <iostream>

namespace blxcpp {

class ThreadPool {
public:

    using Pool = ThreadPool;
    using Task = std::function<void()>;

private:

    // 同步队列
    std::mutex m_queue_lock;
    std::condition_variable m_queue_not_empty;
    std::queue<Task> m_task_queue;

    bool m_stop_signal;
    std::atomic<int> m_busy_threads;

    // 线程池
    std::vector<std::thread> m_threads;

    // 具体的执行函数
    static inline void run(ThreadPool* pool) {
        while(!pool->m_stop_signal) {
            auto task = pool->get();
            if (!pool->m_stop_signal) { break; }
            pool->m_busy_threads++;
            task();
            pool->m_busy_threads--;
        }
        std::cout << "线程结束" << std::endl;
    }

    inline Task get(){
        std::unique_lock<std::mutex> locker(m_queue_lock);
        m_queue_not_empty.wait(locker, [this](){
            return !this->m_task_queue.empty()
                    || this->m_stop_signal;
        });

        if (this->m_stop_signal) {
            return [](){};
        } else {
            auto task = m_task_queue.front();
            m_task_queue.pop();
            return task;
        }
    }

public:

    inline void put(Task&& task) {
        {
            std::lock_guard<std::mutex> sp (m_queue_lock);
            m_task_queue.push(std::forward<Task>(task));
        }
        m_queue_not_empty.notify_one();
    }

    inline bool busy() {
        std::lock_guard<std::mutex> sp (m_queue_lock);
        return m_busy_threads > 0 || !m_task_queue.empty();
    }

    inline void stop() {
        {
            std::lock_guard<std::mutex> sp(m_queue_lock);
            m_stop_signal = true;
        }
        for (auto& thread : m_threads) {
            m_queue_not_empty.notify_all();
            thread.join();
        }
    }

    inline ThreadPool(size_t size = std::thread::hardware_concurrency())
        : m_stop_signal(false)
        , m_busy_threads(0) {
        for (size_t i = 0; i < size; i++) {
            m_threads.push_back(std::thread(&run, this));
        }
    }

};

}

#endif // BLXCPP_THREADPOOL_HPP
