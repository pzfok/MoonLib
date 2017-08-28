#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <netdb.h>      // hostent

#include <iostream>
#include <map>
#include <list>
#include <mutex>
#include <thread>
#include <queue>

#include <libco/co_routine.h>
#include "gtest/gtest.h"

#include "global.h"

#include <CppString.h>
#include <CppArray.h>
#include <CppTime.h>
#include <CppNet.h>

using namespace std;

static const uint32_t SERVER_THREAD_COUNT = 1;              // 服务端线程数，仅在SERVER_TYPE=MULTI_THREAD_SERVER时有效

#if 0
// 服务端工作线程
class ServerWorkThead
{
public:
    ServerWorkThead(uint32_t threadId) : mEpollManager(100), mThreadId(threadId)
    {
    }

    virtual int32_t ProcWrite(epoll_event &inevent);
    virtual int32_t ProcRead(epoll_event &inevent);
    virtual void ProcDeleteFd(int fd);

    void Run();

    void AddClientFd(int clientFd);

private:
    CppEpollManager mEpollManager;
    uint32_t mThreadId;
    map<int, uint64_t> mClientDatas;                         // <客户端fd,数据>
    mutex mLock;
};

/** 服务端处理写请求
*
* @param   epoll_event & inevent
* @param   int epfd
* @retval  int32_t
* @author  moon
*/
int32_t ServerWorkThead::ProcWrite(epoll_event &inevent)
{
    // 构造应答包
    uint64_t rspValue = mClientDatas[inevent.data.fd] + 1;
    rspValue = CppNet::Htonll(rspValue);

    // 回复应答包
    int32_t ret = write(inevent.data.fd, &rspValue, sizeof(rspValue));
    if (ret < 0)
    {
        DEBUG_LOG("write error,event[%u],ret[%d],errno[%d],error[%s].", inevent.events, ret, errno, strerror(errno));
        return ret;
    }

    return 0;
}

/** 服务端处理读请求
*
* @param   epoll_event & inevent
* @retval  int32_t
* @author  moon
*/
int32_t ServerWorkThead::ProcRead(epoll_event &inevent)
{
    char buf[BUF_SIZE];

    // 读取请求
    int32_t ret = read(inevent.data.fd, buf, sizeof(buf));
    if (ret < 0)
    {
        DEBUG_LOG("read error,ret[%d],fd[%d],errno[%d],error[%s].", ret, inevent.data.fd, errno, strerror(errno));
        return ret;
    }

    if (ret == 0)
    {
        DEBUG_LOG("read=0.");
        return -1;
    }

    // 先保存成指针，再解引用，避免产生warning: dereferencing type-punned pointer will break strict-aliasing rules [-Wstrict-aliasing]
    uint64_t *pValue = reinterpret_cast<uint64_t *>(buf);
    mClientDatas[inevent.data.fd] = *pValue;
    mClientDatas[inevent.data.fd] = CppNet::Ntohll(mClientDatas[inevent.data.fd]);

    return 0;
}

void ServerWorkThead::ProcDeleteFd(int fd)
{
    close(fd);
    mClientDatas.erase(fd);
}

void ServerWorkThead::AddClientFd(int clientFd)
{
    // 添加进epoll fd里
    epoll_event ev;
    ev.data.fd = clientFd;
    ev.events = EPOLLIN;

    mEpollManager.AddOrModFd(clientFd, ev);
}

void ServerWorkThead::Run()
{
    int waitTime = 0;
    function<int32_t(epoll_event &inevent)> readFunc = bind(&ServerWorkThead::ProcRead, this, placeholders::_1);
    function<int32_t(epoll_event &inevent)> writeFunc = bind(&ServerWorkThead::ProcWrite, this, placeholders::_1);
    function<void(int fd)> deleteFdFunc = bind(&ServerWorkThead::ProcDeleteFd, this, placeholders::_1);

    while (!gServerStop)
    {
        mEpollManager.Wait(readFunc, writeFunc, deleteFdFunc, waitTime);
    }

    DEBUG_LOG("exit thread[%u].", mThreadId);
}

// 多线程服务端
//  监听线程：1个，用于accept新的客户端连接，获得的连接平均分发到各个数据处理线程（TODO：这里可以使用多种分配模型，比如根据负载，根据客户端数等等）
//  数据处理线程：N个，N可配置，每个线程管理一个Epoll池
//
// TODO：2个模型，选择哪个呢？
//  1、工作线程线程处理连接，主线程只把accept到的fd传给Server线程
//  2、工作线程线程只处理数据，主线程来读取数据
//  先实现第一个看看吧
class MultiThreadServer
{
public:
    /** 启动Server
    *
    * @retval  int32_t
    * @author  moon
    */
    int32_t StartServer();

protected:

    /** 创建服务器监听端口
    *
    * @retval  int
    * @author  moon
    */
    int CreateServerSocket();

    /** 创建服务端Epoll池
    *
    * @param   int serverFd
    * @retval  int
    * @author  moon
    */
    int CreateEpollFd(int serverFd);
};

int MultiThreadServer::CreateServerSocket()
{
    // 创建监听Socket
    int serverFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CHECK_RETURN_F(&cppLog, serverFd >= 0, serverFd, CppLog::ERROR, "socket失败,errno[%d],error[%s].", errno, strerror(errno));
    UniqueFd uniqServerFd(serverFd);

    // 设置监听端口非阻塞
    int flags = fcntl(serverFd, F_GETFL, 0);
    int32_t ret = fcntl(serverFd, F_SETFL, flags | O_NONBLOCK);
    ERROR_RETURN_F(&cppLog, ret, CppLog::ERROR, "fcntl失败,errno[%d],error[%s].", errno, strerror(errno));

    // 设置REUSEADDR标识，服务器重启可以快速使用这个端口，避免在TIME_WAIT状态无法重新监听这个端口
    flags = 1;
    ret = setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags));
    ERROR_RETURN_F(&cppLog, ret, CppLog::ERROR, "setsockopt SO_REUSEADDR失败,errno[%d],error[%s].", errno, strerror(errno));

    flags = 1;
    ret = setsockopt(serverFd, IPPROTO_TCP, TCP_NODELAY, (char *)&flags, sizeof(flags));
    ERROR_RETURN_F(&cppLog, ret, CppLog::ERROR, "setsockopt TCP_NODELAY失败,errno[%d],error[%s].", errno, strerror(errno));

    // 监听端口设置
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERV_PORT);
    servAddr.sin_addr.s_addr = htons(INADDR_ANY);

    // 绑定Socket到端口
    ret = bind(serverFd, (struct sockaddr*)&servAddr, sizeof(servAddr));
    ERROR_RETURN_F(&cppLog, ret, CppLog::ERROR, "bind失败,errno[%d],error[%s].", errno, strerror(errno));

    // 开始监听
    const uint32_t LISTEN_BACKLog = 10;
    ret = listen(serverFd, LISTEN_BACKLog);
    ERROR_RETURN_F(&cppLog, ret, CppLog::ERROR, "listen失败,errno[%d],error[%s].", errno, strerror(errno));

    DEBUG_LOG("服务启动成功,监听端口[%u].", SERV_PORT);
    gServerStart = true;

    return uniqServerFd.Release();
}

int MultiThreadServer::CreateEpollFd(int serverFd)
{
    int epollFd = epoll_create(SERVER_EPOLL_SIZE);
    CHECK_RETURN_F(&cppLog, epollFd >= 0, epollFd, CppLog::ERROR, "epoll_create失败,errno[%d],error[%s].", errno, strerror(errno));
    UniqueFd uniqEpollFd(epollFd);

    // 将监听的端口加入到Epoll池管理
    epoll_event ev;
    ev.data.fd = serverFd;
    ev.events = EPOLLIN | EPOLLRDHUP;
    int32_t ret = epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &ev);
    ERROR_RETURN_F(&cppLog, ret, CppLog::ERROR, "epoll_ctl失败,errno[%d],error[%s].", errno, strerror(errno));

    return uniqEpollFd.Release();
}

int32_t MultiThreadServer::StartServer()
{
    map<int, UniqueFd> clientFds;

    // 忽略SIGPIPE信号，防止客户端关闭后，服务端往Socket中写入数据导致服务端收到此信号导致服务挂掉
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);

    int serverFd = CreateServerSocket();
    CHECK_RETURN(&cppLog, serverFd >= 0, serverFd, CppLog::ERROR);
    UniqueFd uniqServerFd(serverFd);

    int epollFd = CreateEpollFd(serverFd);
    CHECK_RETURN(&cppLog, epollFd >= 0, epollFd, CppLog::ERROR);
    UniqueFd uniqEpollFd(epollFd);

    epoll_event events[SERVER_EPOLL_SIZE];
    sockaddr_in cliAddr;
    socklen_t addrLen = sizeof(cliAddr);

    // 创建工作线程
    list<thread> threads;
    vector<shared_ptr<ServerWorkThead>> workThreads;
    for (uint32_t i = 0; i < SERVER_THREAD_COUNT; ++i)
    {
        workThreads.push_back(make_shared<ServerWorkThead>(i));
        threads.push_back(thread(&ServerWorkThead::Run, workThreads[i].get()));
    }

    // 动态调整wait的时间，有数据的话，则缩短时间
    int waitTime = WAIT_TIME_MS;
    while (!gServerStop)
    {
        int fdsCount = epoll_wait(epollFd, events, ARRAY_SIZE(events), waitTime);
        waitTime = WAIT_TIME_MS;
        CHECK_CONTINUE_F(&cppLog, fdsCount >= 0, fdsCount, CppLog::ERROR, "epoll_wait错误,errno[%d],error[%s].",
                         errno, strerror(errno));

        if (fdsCount == 0)
        {
            // DEBUG_LOG("epoll wait timeout.");
            continue;
        }

        // 读取到客户端新连接，只有一个FD？
        // 修改下次wait的时间为0ms，避免有客户端的时候继续等待
        waitTime = 0;
        if (events[0].events & EPOLLIN)
        {
            int clientFd = accept(events[0].data.fd, (struct sockaddr*)&cliAddr, &addrLen);
            CHECK_CONTINUE_F(&cppLog, clientFd >= 0, fdsCount, CppLog::ERROR, "accept错误,errno[%d],error[%s].",
                             errno, strerror(errno));

            // 设置数据链接非阻塞，测试结果表示对程序的性能没有明显影响。
            //          int flags = fcntl(clientFd, F_GETFL, 0);
            //          fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
            // 
            //          flags = 1;
            //          int ret = setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, (char *)&flags, sizeof(flags));
            //          if (ret != 0)
            //          {
            //              DEBUG_LOG("set TCP_NODELAY failed.");
            //              close(clientFd);
            //              continue;
            //          }

            static uint32_t clientId = 0;
            workThreads[clientId % SERVER_THREAD_COUNT]->AddClientFd(clientFd);
            clientFds[clientFd] = UniqueFd(clientFd);
            DEBUG_LOG("New client[%u], fd[%d], Server thread[%d].",
                      clientId, clientFd, clientId % SERVER_THREAD_COUNT);
            ++clientId;
        }
    }

    for (auto &th : threads)
    {
        th.join();
    }

    return 0;
}



#endif


static void *TestRoutine(void *arg)
{
    uint32_t &num = *static_cast<uint32_t *>(arg);
    ++num;

    // 使用co_yield挂起当前协程
    co_yield(co_self());
    ++num;

    // 使用co_yield_ct挂起当前协程
    co_yield_ct();

    ++num;

    // 这里函数结束，意味着协程也结束了
    return NULL;
}

// 基础协程切换测试：创建N个协程，然后去协程中执行，给num+1，协程中途会出来一次，再进去一次，再给num+1
TEST(LibcoTest, Basic)
{
    static uint32_t COUNT = 100;
    vector<stCoRoutine_t *> cos(COUNT, NULL);
    uint32_t num = 0;
    uint32_t num_ex = 0;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        co_create(&cos[i], NULL, TestRoutine, &num);
        co_resume(cos[i]);
        ++num_ex;
        EXPECT_EQ(num_ex, num);
        co_resume(cos[i]);
        ++num_ex;
        EXPECT_EQ(num_ex, num);
    }

    // 再循环一轮，因为此时协程还没结束
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        co_resume(cos[i]);
        ++num_ex;
        EXPECT_EQ(num_ex, num);

        // 此时协程已经结束了，因此不能再切换过去，否则会Core掉：
        // co_resume(cos[i]);
    }
}

struct CoCtx
{
    int pipe_c2s_fd[2];
    int pipe_s2c_fd[2];
};

// 协程读写测试
static uint32_t RW_COUNT = 1000;
void *CoServerFunc(void *arg)
{
    co_enable_hook_sys();

    cout << "执行" << __FUNCTION__ << endl;
    CoCtx &ctx = *static_cast<CoCtx *>(arg);

    int32_t a = 0;
    int ret;
    while (a != RW_COUNT)
    {
        cout << "准备读取" << __FUNCTION__ << ctx.pipe_c2s_fd[0] << endl;
        ret = read(ctx.pipe_c2s_fd[0], &a, sizeof(a));
        if (ret != 0)
        {
            cout << "read,ret=" << ret << endl;
        }
        cout << "读取完毕" << __FUNCTION__ << ctx.pipe_c2s_fd[0] << "->" << a << endl;
        a += a;
        cout << "准备写入" << __FUNCTION__ << ctx.pipe_s2c_fd[1] << endl;
        ret = write(ctx.pipe_s2c_fd[1], &a, sizeof(a));
        if (ret != 0)
        {
            cout << "read,ret=" << ret << endl;
        }
        cout << "写入完毕" << __FUNCTION__ << ctx.pipe_s2c_fd[1] << "->" << a << endl;
        poll(NULL, 0, 1000);
    }

    return NULL;
}

void *CoClientFunc(void *arg)
{
    co_enable_hook_sys();

    cout << "执行" << __FUNCTION__ << endl;
    CoCtx &ctx = *static_cast<CoCtx *>(arg);
    int32_t a = 0;
    int32_t new_a;
    int ret;

    for (int32_t i = 0; i < RW_COUNT; ++i)
    {
        cout << "准备写入" << __FUNCTION__ << ctx.pipe_c2s_fd[1] << endl;
        ret = write(ctx.pipe_c2s_fd[1], &a, sizeof(a));
        if (ret != 0)
        {
            cout << "read,ret=" << ret << endl;
        }
        cout << "写入完毕" << __FUNCTION__ << ctx.pipe_c2s_fd[1] << "->" << a << endl;

        cout << "准备读取" << __FUNCTION__ << ctx.pipe_c2s_fd[1] << endl;
        ret = read(ctx.pipe_s2c_fd[0], &new_a, sizeof(new_a));
        if (ret != 0)
        {
            cout << "read,ret=" << ret << endl;
        }
        cout << "读取完毕" << __FUNCTION__ << ctx.pipe_c2s_fd[1] << "->" << new_a << endl;
        EXPECT_EQ(new_a, a + a);
        a++;
        cout << a << endl;
        poll(NULL, 0, 1000);
    }

    return NULL;
}

TEST(LibcoTest, WriteRead)
{
    //     int p1[2];
    //     int p2[2];
    // 
    //     pipe(p1);
    //     pipe(p2);

    int flags;

    //     flags = fcntl(p1[0], F_GETFL, 0);
    //     fcntl(p1[0], F_SETFL, flags | O_NONBLOCK);
    // 
    //     flags = fcntl(p1[1], F_GETFL, 0);
    //     fcntl(p1[1], F_SETFL, flags | O_NONBLOCK);

    //     flags = fcntl(p2[0], F_GETFL, 0);
    //     fcntl(p2[0], F_SETFL, flags | O_NONBLOCK);

    //     flags = fcntl(p2[1], F_GETFL, 0);
    //     fcntl(p2[1], F_SETFL, flags | O_NONBLOCK);

    int32_t a = 10;
    //     int32_t b = 20;
    //     int count = write(p1[1], &a, sizeof(a));
    //     cout << "count:" << count << endl;
    //     count = write(p2[1], &b, sizeof(a));
    //     cout << "count:" << count << endl;
    // 
    //     read(p1[0], &b, sizeof(b));
    //     cout << "count:" << count << endl;
    //     cout << b << endl;
    //     read(p2[0], &b, sizeof(b));
    //     cout << "count:" << count << endl;
    //     cout << b << endl;
    //     return;

    CoCtx ctx;
    int32_t ret = pipe(ctx.pipe_c2s_fd);
    if (ret != 0)
    {
        cout << CppString::GetArgs("pipe调用失败,ret[%d]", ret);
        return;
    }

    ret = pipe(ctx.pipe_s2c_fd);
    if (ret != 0)
    {
        cout << CppString::GetArgs("pipe调用失败,ret[%d]", ret);
        return;
    }

    //     flags = fcntl(ctx.pipe_c2s_fd[0], F_GETFL, 0);
    //     fcntl(ctx.pipe_c2s_fd[0], F_SETFL, flags | O_NONBLOCK);
    //     flags = fcntl(ctx.pipe_c2s_fd[1], F_GETFL, 0);
    //     fcntl(ctx.pipe_c2s_fd[1], F_SETFL, flags | O_NONBLOCK);
    //     flags = fcntl(ctx.pipe_s2c_fd[0], F_GETFL, 0);
    //     fcntl(ctx.pipe_s2c_fd[0], F_SETFL, flags | O_NONBLOCK);
    //     flags = fcntl(ctx.pipe_s2c_fd[1], F_GETFL, 0);
    //     fcntl(ctx.pipe_s2c_fd[1], F_SETFL, flags | O_NONBLOCK);

    //     cout << ctx.pipe_c2s_fd[0] << ctx.pipe_c2s_fd[1] << ctx.pipe_s2c_fd[0] << ctx.pipe_s2c_fd[1] << endl;
    //     a = 10;
    //     write(ctx.pipe_c2s_fd[1], &a, sizeof(a));
    //     read(ctx.pipe_s2c_fd[0], &a, sizeof(a));
    //     cout << a << endl;
    //     return;

    stCoRoutine_t *p_client_co = NULL;
    stCoRoutine_t *p_server_co = NULL;
    co_create(&p_server_co, NULL, CoServerFunc, &ctx);
    co_resume(p_server_co);

    co_create(&p_client_co, NULL, CoClientFunc, &ctx);
    co_resume(p_client_co);

    co_eventloop(co_get_epoll_ct(), NULL, NULL);
}


struct stTask_t
{
    int id;
};
struct stEnv_t
{
    stCoCond_t* cond;
    queue<stTask_t*> task_queue;
};
void* Producer(void* args)
{
    co_enable_hook_sys();
    stEnv_t* env = (stEnv_t*)args;
    int id = 0;
    while (true)
    {
        stTask_t* task = (stTask_t*)calloc(1, sizeof(stTask_t));
        task->id = id++;
        env->task_queue.push(task);
        printf("%s:%d produce task %d\n", __func__, __LINE__, task->id);
        co_cond_signal(env->cond);
        poll(NULL, 0, 1000);
    }
    return NULL;
}
void* Consumer(void* args)
{
    co_enable_hook_sys();
    stEnv_t* env = (stEnv_t*)args;
    while (true)
    {
        if (env->task_queue.empty())
        {
            co_cond_timedwait(env->cond, -1);
            continue;
        }
        stTask_t* task = env->task_queue.front();
        env->task_queue.pop();
        printf("%s:%d consume task %d\n", __func__, __LINE__, task->id);
        free(task);
    }
    return NULL;
}

TEST(LibcoTest, Cond)
{
    stEnv_t* env = new stEnv_t;
    env->cond = co_cond_alloc();

    stCoRoutine_t* consumer_routine;
    co_create(&consumer_routine, NULL, Consumer, env);
    co_resume(consumer_routine);

    stCoRoutine_t* producer_routine;
    co_create(&producer_routine, NULL, Producer, env);
    co_resume(producer_routine);

    co_eventloop(co_get_epoll_ct(), NULL, NULL);
}

static const string SERV_NAME = "127.0.0.1";
static const uint16_t SERV_PORT = 20001;

// 协程读写TCP测试
int CreateServerSocket()
{
    int32_t ret = 0;

    // 创建监听Socket
    int serverFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverFd < 0)
    {
        // DEBUG_LOG("Create socket failed.");
        return serverFd;
    }

    // 设置监听端口非阻塞
    int flags = fcntl(serverFd, F_GETFL, 0);
    ret = fcntl(serverFd, F_SETFL, flags | O_NONBLOCK);
    if (ret < 0)
    {
        // DEBUG_LOG("fcntl failed.");
        close(serverFd);
        return ret;
    }

    // 设置REUSEADDR标识，服务器重启可以快速使用这个端口，避免在TIME_WAIT状态无法重新监听这个端口
    int rep = 1;
    ret = setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &rep, sizeof(rep));
    if (ret < 0)
    {
        // DEBUG_LOG("set reuse addr failed.");
        close(serverFd);
        return ret;
    }

    // 监听端口设置
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERV_PORT);
    servAddr.sin_addr.s_addr = htons(INADDR_ANY);

    // 绑定Socket到端口
    ret = bind(serverFd, (struct sockaddr*)&servAddr, sizeof(servAddr));
    if (ret < 0)
    {
        // DEBUG_LOG("bind failed.");
        close(serverFd);
        return ret;
    }

    // 开始监听
    const uint32_t LISTEN_BACKLog = 10;
    ret = listen(serverFd, LISTEN_BACKLog);
    if (ret < 0)
    {
        //  DEBUG_LOG("listen failed.");
        close(serverFd);
        return ret;
    }

    // DEBUG_LOG("Begin to listen port[%u]... ", SERV_PORT);

    return serverFd;
}

int co_accept(int fd, struct sockaddr *addr, socklen_t *len);
void *CoServerFuncTcp(void *arg)
{
    co_enable_hook_sys();
    int ret;

    int listen_fd = CreateServerSocket();
    cout << "listen:" << listen_fd << endl;

    sockaddr_in cliAddr;
    socklen_t addrLen = sizeof(cliAddr);

    int clientFd = -1;
    while (clientFd == -1)
    {
        clientFd = co_accept(listen_fd, (struct sockaddr*)&cliAddr, &addrLen);

        if (clientFd < 0)
        {
            struct pollfd pf = { 0 };
            pf.fd = listen_fd;
            pf.events = (POLLIN | POLLERR | POLLHUP);
            co_poll(co_get_epoll_ct(), &pf, 1, 1000);
            continue;
        }
    }

    int flags = fcntl(clientFd, F_GETFL, 0);
    ret = fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

    // 进行读写
    int32_t data = 0;
    struct pollfd pf = { 0 };
    pf.fd = clientFd;
    pf.events = (POLLIN | POLLERR | POLLHUP);

    while (data < RW_COUNT)
    {
        ret = read(clientFd, &data, sizeof(data));
        if (ret < 0)
        {
            pf.events = (POLLIN | POLLERR | POLLHUP);
            co_poll(co_get_epoll_ct(), &pf, 1, 1000);
            continue;
        }

        ++data;
        ret = write(clientFd, &data, sizeof(data));
        if (ret < 0)
        {
            pf.events = (POLLOUT | POLLERR | POLLHUP);
            co_poll(co_get_epoll_ct(), &pf, 1, 1000);
            continue;
        }
    }

    return NULL;
}

void *CoClientFuncTcp(void *arg)
{
    co_enable_hook_sys();
    string mServerAddr = "127.0.0.1";
    int16_t mServerPort = SERV_PORT;

    char buf[1024];
    int ret;

    // 解析域名，如果是IP则不用解析，如果出错，显示错误信息
    hostent nlp_host, *p_host;
    gethostbyname_r(mServerAddr.c_str(), &nlp_host, buf, sizeof(buf), &p_host, &ret);
    //CHECK_RETURN_F(mpCppLog, ret == 0, -1, CppLog::ERROR, "gethostbyname失败,errno[%d],error[%s].", errno, strerror(errno));

    // 设置pin变量，包括协议、地址、端口等，此段可直接复制到自己的程序中
    sockaddr_in pin;
    bzero(&pin, sizeof(pin));
    pin.sin_family = AF_INET;                   // AF_INET表示使用IPv4
    pin.sin_addr.s_addr = ((struct in_addr *)(nlp_host.h_addr))->s_addr;       // TODO:这一句是干嘛用的？好像没什么用，去掉不影响。
    pin.sin_port = htons(mServerPort);

    // 建立socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    //CHECK_RETURN_F(mpCppLog, fd >= 0, fd, CppLog::ERROR, "socket失败,ret[%d],errno[%d],error[%s].", fd, errno, strerror(errno));

    // 中途出错，释放FD
    UniqueFd uniqFd(fd);

    // 建立连接
    ret = connect(fd, (struct sockaddr*)&pin, sizeof(pin));
    //ERROR_RETURN_F(mpCppLog, ret, CppLog::ERROR, "connect失败,errno[%d],error[%s].", errno, strerror(errno));

    int flags = fcntl(fd, F_GETFL, 0);
    ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    //ERROR_RETURN_F(mpCppLog, ret, CppLog::ERROR, "fcntl O_NONBLOCK失败,errno[%d],error[%s].", errno, strerror(errno));

    flags = 1;
    ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flags, sizeof(flags));
    //ERROR_RETURN_F(mpCppLog, ret, CppLog::ERROR, "setsockopt TCP_NODELAY失败,errno[%d],error[%s].", errno, strerror(errno));

    cout << "执行" << __FUNCTION__ << endl;
    CoCtx &ctx = *static_cast<CoCtx *>(arg);
    int32_t a = 0;
    int32_t new_a;

    for (int32_t i = 0; i < RW_COUNT; ++i)
    {
        cout << "准备写入" << __FUNCTION__ << ctx.pipe_c2s_fd[1] << endl;
        ret = write(ctx.pipe_c2s_fd[1], &a, sizeof(a));
        if (ret < 0)
        {
            cout << "write错误:" << ret << endl;
        }
        cout << "写入" << __FUNCTION__ << ctx.pipe_c2s_fd[1] << endl;

        ret = read(ctx.pipe_s2c_fd[0], &new_a, sizeof(new_a));
        if (ret < 0)
        {
            cout << "read错误:" << ret << endl;
        }
        EXPECT_EQ(new_a, a + 1);
        a = new_a;
        cout << a << endl;
    }

    return NULL;
}

// 协程读写测试
TEST(LibcoTest, WriteReadTcp)
{
    CoCtx ctx;
    //     int32_t ret = pipe(ctx.pipe_c2s_fd);
    //     if (ret != 0)
    //     {
    //         cout << CppString::GetArgs("pipe调用失败,ret[%d]", ret);
    //         return;
    //     }
    // 
    //     ret = pipe(ctx.pipe_s2c_fd);
    //     if (ret != 0)
    //     {
    //         cout << CppString::GetArgs("pipe调用失败,ret[%d]", ret);
    //         return;
    //     }

    stCoRoutine_t *p_server_co = NULL;
    co_create(&p_server_co, NULL, CoServerFuncTcp, &ctx);
    co_resume(p_server_co);

    stCoRoutine_t *p_client_co = NULL;
    co_create(&p_client_co, NULL, CoClientFuncTcp, &ctx);
    co_resume(p_client_co);

    co_eventloop(co_get_epoll_ct(), NULL, NULL);
}
