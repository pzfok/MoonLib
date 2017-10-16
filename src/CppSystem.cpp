#include "CppSystem.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <CppFile.h>

using namespace std;

string CppSystem::ExcuteCommand(const string &cmd, int32_t &ret)
{
    FILE *fp = popen(cmd.c_str(), "r");
    if (fp == NULL)
    {
        THROW("popen return NULL");
    }

    char buf[BUFSIZ];
    string result;
    int32_t bufSize;

    while (!feof(fp))
    {
        bufSize = fread(buf, 1, sizeof(buf), fp);
        if (bufSize < 0)
        {
            break;
        }

        result.append(buf, bufSize);
    }

    ret = pclose(fp);
    ret = WEXITSTATUS(ret);

    return result;
}

std::vector<double> CppSystem::GetLoadAvg()
{
    // 获得总负载信息:"3.31 4.35 4.56 5/428 119681"
    vector<string> loadDataSplitResult;
    string loadavg_str = CppFile::ReadFromFile("/proc/loadavg");
    CppString::SplitStr(loadavg_str, " ", loadDataSplitResult);

    if (loadDataSplitResult.size() < 3)
    {
        THROW("读取CPU负载失败，文件格式[%s]不正确.", loadavg_str.c_str());
    }

    uint32_t cpu_num = GetCpuNum();
    return{ CppString::FromString<double>(loadDataSplitResult[0]) / cpu_num,
        CppString::FromString<double>(loadDataSplitResult[1]) / cpu_num,
        CppString::FromString<double>(loadDataSplitResult[2]) / cpu_num };
}

uint32_t CppSystem::GetCpuNum()
{
    // 获得CPU核数，读取/proc/cpuinfo，取最后一个CPU编号+1
    ifstream ifs("/proc/cpuinfo");
    string line;
    string cpuLine;       // 记载了CPU信息的最后一行
    while (getline(ifs, line))
    {
        if (line.find("processor") == 0)
        {
            cpuLine = line;
        }
    }

    vector<string> cpuLineSplitResult;
    CppString::SplitStr(cpuLine, " ", cpuLineSplitResult);
    if (cpuLineSplitResult.size() != 2)
    {
        THROW("读取CPU核数失败，文件格式[%s]不正确.", cpuLine.c_str());
    }

    return CppString::FromString<uint32_t>(cpuLineSplitResult[1]) + 1;
}

void CppSystem::InitDaemon()
{
    int pid;
    int i;
    if ((pid = fork()))
    {
        // 是父进程,结束父进程
        exit(0);
    }
    else if (pid < 0)
    {
        // fork失败,退出
        exit(-1);
    }
    // 是第一子进程,后台继续执行

    // 第一子进程成为新的会话组长和进程组长
    setsid();

    // 并与控制终端分离
    if ((pid = fork()))
    {
        // 是第一子进程,结束第一子进程
        exit(0);
    }
    else if (pid < 0)
    {
        // fork失败,退出
        exit(-1);
    }

    // 是第二子进程,继续
    // 第二子进程不再是会话组长
    // 关闭打开的文件描述符
    for (i = 0; i < NOFILE; ++i)
    {
        close(i);
    }

    // 改变工作目录到/tmp
    if (chdir("/tmp"))
    {
        int32_t errnoBak = errno;
        THROW("chdir失败,errno[%d],err[%s].", errnoBak, strerror(errnoBak));
    }

    // 重设文件创建掩模
    umask(0);
}
