#include <thread>
#include <mutex>
#include <iostream>
#include <list>
#include <map>
#include <condition_variable>
#include <atomic>

#include "gtest/gtest.h"

#include <CppMath.h>
#include <CppLog.h>
#include <CppThread.h>

#include "global.h"

using namespace std;

/************************************************************************/
/* 单生产者-多消费者模型                                                */
/************************************************************************/
static const uint64_t COUNT = 20000;        // 总共计数数量
static const uint64_t THREAD_COUNT = 2000;    // 线程数

uint64_t CurrValue;                         // 当前生产的值（生产者与消费者之间的消息传递），为0表示数据可以生产
uint64_t SumValue;                          // 消费者的输出值，所有生产值的和（多个消费者之间的消息传递）
bool Stop = false;                          // 是否停止
bool Start = false;                         // 是否开始

mutex ValueMutex;                           // 临界区互斥量
condition_variable SetCV;                   // 条件变量指示：是否可生产
condition_variable GetCV;                   // 条件变量指示：是否可读取

mutex DataMutex;                            // 消费者产出数据保护互斥量

static bool ShowProcInfo = false;           // 是否显示处理过程中的信息

// 生产者
void SetFun()
{
    for (uint64_t i = 1; i <= COUNT; ++i)
    {
        // 等待可生产信号
        unique_lock<mutex> setLock(ValueMutex);
        SetCV.wait(setLock, []
        {
            return CurrValue == 0;      // 等待，直到CurrValue为0，表示数据已经被取走
        });

        // 生产一个数据，将其存入CurrValue，供消费者读取
        CurrValue = i;

        if (ShowProcInfo)
        {
            cout << "生产" << i << "-----------" << endl;
        }
        // 通知一个消费者读取数据
        GetCV.notify_one();

        // setLock在此处析构会调用ValueMutex.unlock()解锁
    }

    // 设置停止标志
    Stop = true;

    // 通知所有的消费者，如果没有这一行，部分消费者会挂在wait中（通过GDB可以查看到）
    GetCV.notify_all();
}

// 消费者
void GetFun(uint32_t threadId)
{
    static_cast<void>(threadId);
    if (ShowProcInfo)
    {
        cout << "线程" << threadId << "启动" << endl;
    }
    uint64_t currValueBak;          // 取出生产者生产的数值临时存放，进行处理
    while (true)
    {
        unique_lock<mutex> getLock(ValueMutex);
        if (ShowProcInfo)
        {
            cout << "线程" << threadId << "抢到锁" << endl;
        }
        Start = true;
        if (ShowProcInfo)
        {
            cout << "线程" << threadId << "即将wait，释放锁" << endl;
        }
        GetCV.wait(getLock, []
        {
            return CurrValue != 0 || Stop;      // 仅当CurrValue不为0或者停止标志生效时，退出wait
        });

        // // 另一种等待形式：
        // while (CurrValue == 0 && !Stop)
        // {
        //     GetCV.wait(getLock);
        // }

        if (ShowProcInfo)
        {
            cout << "线程" << threadId << "wait后,value=" << CurrValue << endl;
        }

        // 用于最后的退出部分
        if (CurrValue == 0 && Stop)
        {
            if (ShowProcInfo)
            {
                cout << "线程" << threadId << "break出去" << endl;
            }
            break;
        }

        // 记录数据后解锁，相当于提取数据
        currValueBak = CurrValue;

        // 设置标志
        CurrValue = 0;

        // 唤醒生产者
        if (ShowProcInfo)
        {
            cout << "线程" << threadId << "通知生产者" << endl;
        }
        SetCV.notify_one();

        if (ShowProcInfo)
        {
            cout << "线程" << threadId << "释放锁，处理" << currValueBak << endl;
        }
        getLock.unlock();

        // 模拟处理数据延时，每个线程单独处理
        usleep(rand() % 20000);

        // 记录数据，需要对共享数据加锁
        unique_lock<mutex> dataLock(DataMutex);
        SumValue += currValueBak;
    }
}

TEST(CppThreadTest, DISABLED_ConditionVariable)
{
    vector<thread> threads;

    // 创建消费线程
    for (uint64_t i = 0; i < THREAD_COUNT; ++i)
    {
        threads.push_back(thread(GetFun, i));
    }

    // 等待信号启动生产者，因为无需太久，此处采用忙等待
    while (!Start);
    SetFun();

    // 等待消费者线程结束
    for (auto &currThread : threads)
    {
        currThread.join();
    }

    // 计算期望数值
    uint64_t expectValue = (1 + COUNT) * COUNT / 2;
    EXPECT_EQ(expectValue, SumValue);
}

TEST(CppThreadTest, Mutex)
{
    volatile int counter(0); // non-atomic counter
    std::mutex mtx;           // locks access to counter

    static const uint32_t THREAD_COUNT = 10;
    static const uint32_t COUNT = 10000;
    std::thread threads[THREAD_COUNT];
    for (uint32_t i = 0; i < THREAD_COUNT; ++i)
        threads[i] = std::thread([&]
    {
        for (uint32_t i = 0; i < COUNT; ++i)
        {
            lock_guard<mutex> lock(mtx);
            // only increase if currently not locked:
            ++counter;
        }
    });

    for (auto& th : threads) th.join();
    std::cout << counter << " successful increases of the counter.\n";
}

shared_ptr<map<uint32_t, uint32_t>> GetData(shared_ptr<map<uint32_t, uint32_t>> &data, mutex &dataLock)
{
    unique_lock<mutex> lock(dataLock);
    return data;
}

// 无锁配置
// 用于一段缓存，读可以缓更新，写可以随时写，但是不加锁也不会冲突的情况
TEST(CppThreadTest, DISABLED_LocklessBufferWithSharedPtr)
{
    static const uint32_t READ_THREAD_COUNT = 30;       // 读线程数量
    static const uint32_t WRITE_THREAD_COUNT = 30;      // 写线程数量
    static const uint32_t TEST_SECOND = 30;             // 测试时间
    static const bool USE_LOCK = true;                 // 是否使用mutex锁

    list<thread> readThreads;
    list<thread> writeThreads;

    mutex dataLock;

    // 这个是数据，用shared_ptr包起来
    shared_ptr<map<uint32_t, uint32_t>> data = make_shared<map<uint32_t, uint32_t>>();
    uint32_t sum = 0;
    uint32_t lockCount = 0;
    bool stop = false;

    for (uint32_t i = 0; i < READ_THREAD_COUNT; ++i)
    {
        thread th([&]()
        {
            while (!stop)
            {
                // 读取数据
                shared_ptr<map<uint32_t, uint32_t>> dataBak;
                if (USE_LOCK)
                {
                    // 加锁版本
//                     dataLock.lock();
//                     dataBak = data;
//                     dataLock.unlock();

                    dataBak = GetData(data, dataLock);
                }
                else
                {
                    // 原子操作版本
                    dataBak = atomic_load(&data);
                }

                ++lockCount;
                for (auto dataIt = dataBak->begin(); dataIt != dataBak->end(); ++dataIt)
                {
                    // 这里只要有for循环就好了，如果数据结构出错，循环挂掉
                    sum += dataIt->first;
                    sum += dataIt->second;
                }
            }
        });
        readThreads.push_back(move(th));
    }

    for (uint32_t i = 0; i < WRITE_THREAD_COUNT; ++i)
    {
        thread th([&]()
        {
            while (!stop)
            {
                // 写入数据，构造一个数据，然后swap掉原数据
                map<uint32_t, uint32_t> dataNew;
                uint32_t count = CppMath::Random(0, 1000);      // 数据量大小
                for (uint32_t j = 0; j < count; ++j)
                {
                    dataNew[j] = j;
                }

                auto dataaaa = make_shared<map<uint32_t, uint32_t>>(move(dataNew));
                if (USE_LOCK)
                {
                    // 加锁版本
                    dataLock.lock();
                    data.swap(dataaaa);
                    dataLock.unlock();
                }
                else
                {
                    // 原子操作版本
                    atomic_exchange(&data, dataaaa);
                    //atomic_store(&data, dataaaa);
                }

                // 此时dataNew应该是没有数据的，因为move掉了
                ASSERT_EQ(static_cast<size_t>(0), dataNew.size());
            }
        });
        readThreads.push_back(move(th));
    }

    // 等待时间到
    for (uint32_t i = 0; i < TEST_SECOND; ++i)
    {
        INFOR_LOG("已经运行[%u]秒,总共[%u]秒,还剩下[%u]秒结束.", i, TEST_SECOND, TEST_SECOND - i);
        sleep(1);
    }

    stop = true;
    for (auto th_it = readThreads.begin(); th_it != readThreads.end(); ++th_it)
    {
        th_it->join();
    }

    for (auto th_it = writeThreads.begin(); th_it != writeThreads.end(); ++th_it)
    {
        th_it->join();
    }

    INFOR_LOG("sum的值为[%u],lockCount[%u].", sum, lockCount);
}

TEST(CppThreadTest, ThreadPool)
{
    ThreadPool thread_pool(10);
    list<shared_ptr<ThreadContext> > tcs;

    // 低于线程数的并发
    for (uint32_t i = 0; i < 3; ++i)
    {
        tcs.emplace_back(thread_pool.Run(bind([](uint32_t sec) {
            sleep(sec);
            ERROR_LOG("%u", sec);
        }, i)));
    }

    for (auto &tc : tcs)
    {
        tc->Wait();
    }

    tcs.clear();

    // 高于线程数的并发，并且测试逆序Wait，这样最后的几个线程，会被前面的线程阻塞住，直到有空闲线程出来
    for (uint32_t i = 0; i < 15; ++i)
    {
        tcs.emplace_back(thread_pool.Run(bind([](uint32_t sec) {
            sleep(sec);
            INFOR_LOG("%u", sec);
        }, 15 - i)));
    }

    for (auto &tc : tcs)
    {
        tc->Wait();
    }
}
