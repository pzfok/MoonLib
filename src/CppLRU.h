#ifndef _CPP_LRU_H_
#define _CPP_LRU_H_

#include <stdint.h>

#include <unordered_set>
#include <list>

// 提供LRU淘汰
template <class T>
class CppLRU
{
public:
    /** 判断元素是否存在
     *
     * @param   const T * t
     * @retval  bool
     * @author  moontan
     */
    bool Exist(const T &t);

    /** 往LRU里添加元素
     *
     * @param   const T & t
     * @retval  void
     * @author  moontan
     */
    void Add(const T &t);

    /** 删除元素
     *
     * @param   const T & t
     * @retval  void
     * @author  moontan
     */
    void Del(const T &t);

    // 清空所有元素
    //
    // @retval  void
    // @author  moon
    void Clear();

    CppLRU(uint32_t maxCap = 100) :mMaxCap(maxCap)
    {
    }

    std::unordered_set<T> mSet;
    std::list<T> mList;
    uint32_t mMaxCap;       // 最大容量
};

template <class T>
void CppLRU<T>::Clear()
{
    mSet.clear();
    mList.clear();
}

template <class T>
void CppLRU<T>::Del(const T &t)
{
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
void CppLRU<T>::Add(const T &t)
{
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
    return mSet.find(t) != mSet.end();
}

#endif
