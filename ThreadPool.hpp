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

    static inline void run(Pool* pool) {
        while(true){

            TaskS task;
            {
                std::unique_lock<std::mutex> locker(pool->m_queue_lock);
                pool->m_queue_not_empty.wait(locker, [pool](){
                    return !pool->m_task_queue.empty();
                });
                task = pool->m_task_queue.front();
                pool->m_task_queue.pop();

                // 获取到 task 了以后就要立即把线程设置为 busy 了
                // 防止出现蛋疼的时间差
                pool->m_busy_threads++;
            }

            bool signal = task();

            // 执行完后再把线程设为 free
            pool->m_busy_threads--;

            if (!signal) break;
        }
    }

    static inline TaskS create(const Task& task, bool signal = true) {
        return [=]{ task(); return signal; };
    }


public:

    inline bool busy() {
        std::lock_guard<std::mutex> sp (m_queue_lock);
        return m_busy_threads > 0 || !m_task_queue.empty();
    }

    inline void put(const Task& task){
        {
            std::lock_guard<std::mutex> sp (m_queue_lock);
            m_task_queue.push(create(task));
        }
        m_queue_not_empty.notify_one();
    }

    inline ThreadPool(size_t init_size = std::thread::hardware_concurrency())
        : m_busy_threads(0) {
        for (size_t i = 0; i < init_size; i++) {
            m_threads.push_back(std::thread(&run, this));
        }
    }

    inline ~ThreadPool() {
        {
            std::lock_guard<std::mutex> sp (m_queue_lock);
            for (size_t i = 0; i < m_threads.size(); i++) {
                m_task_queue.push(create([]{}, false));
            }
        }

        m_queue_not_empty.notify_all();

        for (std::thread& thread : m_threads) {
            thread.join();
        }
    }

};

}

#endif // BLXCPP_THREADPOOL_HPP
