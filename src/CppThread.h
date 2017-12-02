#pragma once

#include <mutex>
#include <memory>
#include <functional>
#include <list>
#include <thread>
#include <condition_variable>

class CppThreadContext
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
class CppThreadPool
{
public:
    CppThreadPool(uint32_t thread_count)
    {
        for (uint32_t i = 0; i < thread_count; ++i)
        {
            m_ths.emplace_back(&CppThreadPool::RunTask, this);
        }
    }

    std::shared_ptr<CppThreadContext> Run(const std::function<void()> &fun, bool high_priority = false);

    // 在同一个context上执行另一个任务，Wait的时候，任意一个任务执行完毕将会获得通知
    //
    // @param   std::shared_ptr<CppThreadContext> context
    // @param   const std::function<void()> & fun
    // @param   bool high_priority
    // @retval  void
    // @author  moon
    void RunOnContext(std::shared_ptr<CppThreadContext> context, const std::function<void()> &fun, bool high_priority = false);

    void Stop();

    uint32_t ThreadCount()
    {
        return m_ths.size();
    }

    ~CppThreadPool();

protected:
    void RunTask();

    std::list<std::pair<std::shared_ptr<const std::function<void()>>, std::shared_ptr<CppThreadContext>>> m_fun_list;    // 待执行任务列表
    std::list<std::thread> m_ths;
    bool m_stop = false;

    std::mutex m_fun_list_mutex;                 // 操作待执行任务列表的锁
    std::condition_variable m_notify_cv;         // 线程唤醒条件变量
};
