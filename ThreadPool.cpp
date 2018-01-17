// ThreadPool.cpp
#include "ThreadPool.hpp"

namespace blxcpp {

void ThreadPool::run(ThreadPool::Pool *pool) {
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

ThreadPool::TaskS ThreadPool::create(const ThreadPool::Task &task, bool signal) {
    return [=]{ task(); return signal; };
}

bool ThreadPool::busy() {
    std::lock_guard<std::mutex> sp (m_queue_lock);
    return m_busy_threads > 0 || !m_task_queue.empty();
}

void ThreadPool::put(const ThreadPool::Task &task){
    {
        std::lock_guard<std::mutex> sp (m_queue_lock);
        m_task_queue.push(create(task));
    }
    m_queue_not_empty.notify_one();
}

ThreadPool::ThreadPool(size_t init_size)
    : m_busy_threads(0) {
    for (size_t i = 0; i < init_size; i++) {
        m_threads.push_back(std::thread(&run, this));
    }
}

ThreadPool::~ThreadPool() {
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


}
