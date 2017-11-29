#pragma once

#include <mutex>
#include <memory>
#include <functional>
#include <list>
#include <thread>
#include <condition_variable>

class ThreadContext
{
public:

    bool m_is_finish = false;
    bool m_is_abort = false;

    void Wait();

    void Finish(bool is_abort = false);

private:
    std::mutex m_notify_mutex;                   // 通知结果唤醒mutex
    std::condition_variable m_notify_cv;         // 通知结果唤醒条件变量
};

// 线程池
class ThreadPool
{
public:
    ThreadPool(uint32_t thread_count)
    {
        for (uint32_t i = 0; i < thread_count; ++i)
        {
            m_ths.emplace_back(&ThreadPool::RunTask, this);
        }
    }

    std::shared_ptr<ThreadContext> Run(const std::function<void()> &fun, bool high_priority = false);

    void Stop();

    uint32_t ThreadCount()
    {
        return m_ths.size();
    }

    ~ThreadPool();

protected:
    void RunTask();

    std::list<std::pair<std::shared_ptr<const std::function<void()>>, std::shared_ptr<ThreadContext>>> m_fun_list;    // 待执行任务列表
    std::mutex m_fun_list_mutex;                 // 操作待执行任务列表的锁
    std::list<std::thread> m_ths;
    bool m_stop = false;

    std::mutex m_notify_mutex;                   // 线程唤醒mutex
    std::condition_variable m_notify_cv;         // 线程唤醒条件变量
};
