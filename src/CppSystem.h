#ifndef _CPP_SYSTEM_H_
#define _CPP_SYSTEM_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "CppLog.h"

using namespace std;

class CppSystem
{
public:
    /** 执行命令，返回结果
     *
     * @param   const string & cmd
     * @retval  string
     * @author  moon
     */
    static string ExcuteCommand(const string &cmd, int32_t &ret);

    /** 获得系统每核负载
     *
     * @retval  std::vector<double>     索引0,1,2分别为1分钟，5分钟，15分钟系统每核平均负载
     * @author  moon
     */
    static std::vector<double> GetLoadAvg();

    /** 获得CPU核数
     *
     * @retval  uint32_t
     * @author  moon
     */
    static uint32_t GetCpuNum();

    // 成为守护进程
    //
    // @retval  void
    // @author  moon
    static void InitDaemon();
};

#endif
