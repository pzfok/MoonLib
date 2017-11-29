#ifndef _CPP_LRU_H_
#define _CPP_LRU_H_

#include <stdint.h>

#include <unordered_set>
#include <list>
#include <mutex>

// 提供LRU淘汰
template <class T>
class CppLRU
{
public:
    /** 判断元素是否存在
     *
     * @param   const T * t
     * @retval  bool
     * @author  moon
     */
    bool Exist(const T &t);

    /** 往LRU队列尾部添加元素
     *
     * @param   const T & t
     * @retval  void
     * @author  moon
     */
    void Push(const T &t);

    // 取出队头元素
    //
    // @retval  bool
    // @author  moon
    bool Pop(T *p_Value);

    /** 删除元素
     *
     * @param   const T & t
     * @retval  void
     * @author  moon
     */
    void Del(const T &t);

    // 清空所有元素
    //
    // @retval  void
    // @author  moon
    void Clear();

    // 判断是否为空
    //
    // @retval  bool
    // @author  moon
    bool Empty();

    CppLRU(uint32_t maxCap = 100, bool needLock = false) :mMaxCap(maxCap), mNeedLock(needLock)
    {
    }

    std::unordered_set<T> mSet;
    std::list<T> mList;
    uint32_t mMaxCap;       // 最大容量
    bool mNeedLock;         // 是否需要锁
    std::mutex mDataMutex;  // 数据锁
};

template <class T>
bool CppLRU<T>::Empty()
{
    shared_ptr<lock_guard<mutex>> p_lock;
    if (mNeedLock)
    {
        p_lock.reset(new lock_guard<mutex>(mDataMutex));
    }

    return mList.empty();
}

template <class T>
bool CppLRU<T>::Pop(T *p_Value)
{
    shared_ptr<lock_guard<mutex>> p_lock;
    if (mNeedLock)
    {
        p_lock.reset(new lock_guard<mutex>(mDataMutex));
    }

    if (!mList.empty())
    {
        // 如果需要获取数据，将数据传出去
        if (p_Value)
        {
            swap(*p_Value, mList.front());
        }

        mSet.erase(mList.front());
        mList.pop_front();

        return true;
    }

    return false;
}

template <class T>
void CppLRU<T>::Clear()
{
    shared_ptr<lock_guard<mutex>> p_lock;
    if (mNeedLock)
    {
        p_lock.reset(new lock_guard<mutex>(mDataMutex));
    }

    mSet.clear();
    mList.clear();
}

template <class T>
void CppLRU<T>::Del(const T &t)
{
    shared_ptr<lock_guard<mutex>> p_lock;
    if (mNeedLock)
    {
        p_lock.reset(new lock_guard<mutex>(mDataMutex));
    }

    if (mSet.erase(t) > 0)
    {
        auto it = find(mList.begin(), mList.end(), t);
        if (it != mList.end())
        {
            mList.erase(it);
        }
    }
}

template <class T>
void CppLRU<T>::Push(const T &t)
{
    shared_ptr<lock_guard<mutex>> p_lock;
    if (mNeedLock)
    {
        p_lock.reset(new lock_guard<mutex>(mDataMutex));
    }

    if (mSet.insert(t).second)
    {
        mList.push_back(t);

        // 如果超过容量，删除掉最老的
        while (mSet.size() > mMaxCap)
        {
            mSet.erase(mList.front());
            mList.pop_front();
        }
    }
}

template <class T>
bool CppLRU<T>::Exist(const T &t)
{
    shared_ptr<lock_guard<mutex>> p_lock;
    if (mNeedLock)
    {
        p_lock.reset(new lock_guard<mutex>(mDataMutex));
    }

    return mSet.find(t) != mSet.end();
}

#endif
