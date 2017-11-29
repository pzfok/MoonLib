#include "CppThread.h"

using namespace std;

void ThreadContext::Wait()
{
    unique_lock<mutex> notify_lock(m_notify_mutex);
    m_notify_cv.wait(notify_lock, [&]
    {
        return m_is_finish;          // 结束时跳出
    });
}

void ThreadContext::Finish(bool is_abort /*= false*/)
{
    m_is_abort = is_abort;
    m_is_finish = true;

    m_notify_cv.notify_one();
}

shared_ptr<ThreadContext> ThreadPool::Run(const function<void()> &fun, bool high_priority /*= false*/)
{
    // 将任务加入队列
    shared_ptr<ThreadContext> thread_context = make_shared<ThreadContext>();

    unique_lock<mutex> list_lock(m_fun_list_mutex);
    if (high_priority)
    {
        m_fun_list.emplace_front(make_shared<const function<void()>>(fun), thread_context);
    }
    else
    {
        m_fun_list.emplace_back(make_shared<const function<void()>>(fun), thread_context);
    }
    list_lock.unlock();

    // 唤醒一个线程
    m_notify_cv.notify_one();
    return thread_context;
}

void ThreadPool::Stop()
{
    m_stop = true;
    m_notify_cv.notify_all();

    for (auto &th : m_ths)
    {
        th.join();
    }

    m_ths.clear();
}

ThreadPool::~ThreadPool()
{
    Stop();
}

void ThreadPool::RunTask()
{
    while (!m_stop)
    {
        unique_lock<mutex> nority_lock(m_notify_mutex);
        m_notify_cv.wait(nority_lock, [&]
        {
            unique_lock<mutex> list_lock(m_fun_list_mutex);
            return !m_fun_list.empty() || m_stop;          // 有数据时跳出
        });
        nority_lock.unlock();

        if (m_stop)
        {
            break;
        }

        // 如果获得执行权利，就一直执行下去，直到队列空，这样可以避免队列太多无法执行到的情况
        while (!m_stop)
        {
            // 取出第一个数据
            unique_lock<mutex> list_lock(m_fun_list_mutex);
            if (m_fun_list.empty() || m_stop)
            {
                break;
            }

            auto fun_data = move(m_fun_list.front());
            m_fun_list.pop_front();

            list_lock.unlock();

            bool abort = true;
            try
            {
                (*fun_data.first)();
                abort = false;
            }
            catch (...)
            {
                // TODO：自定义异常函数处理？
            }

            fun_data.second->Finish(abort);
        }
    }

    // 对所有的任务执行终止
    unique_lock<mutex> list_lock(m_fun_list_mutex);
    while (!m_fun_list.empty())
    {
        m_fun_list.front().second->Finish(true);
    }
}
