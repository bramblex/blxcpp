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
    using TaskS = std::function<bool()>;

    std::mutex m_queue_lock;
    std::condition_variable m_queue_not_empty;
    std::queue<TaskS> m_task_queue;

    std::vector<std::thread> m_threads;
    std::atomic<int> m_busy_threads;

private:

    static void run(Pool* pool);
    static TaskS create(const Task& task, bool signal = true);

public:

    bool busy();
    void put(const Task& task);
    ThreadPool(size_t init_size = std::thread::hardware_concurrency());
    ~ThreadPool();

};

}

#endif // BLXCPP_THREADPOOL_HPP
