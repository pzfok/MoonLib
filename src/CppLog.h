#ifndef _CPP_LOG_H_
#define _CPP_LOG_H_

#include <stdint.h>
#include <string.h>

#include <fstream>
#include <string>
#include <ostream>

#include "CppTime.h"
#include "CppString.h"

#ifdef WIN32
#define __builtin_expect(x,b) x
#endif

#ifndef likely
#define likely(x)       __builtin_expect((x),1)
#endif

#ifndef unlikely
#define unlikely(x)     __builtin_expect((x),0)
#endif

#define CURR_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define FILE_LOCATION (std::string(CURR_FILENAME) + ":" + CppString::ToString(__LINE__))

// TODO：
// 如果日志量过大，需要缓存，采用定时写入或者定量写入，不能每次都打开文件写入再关闭
// 重要的日志，比如ERROR和WARNN级别的，必要的时候需要flush到磁盘

/* 为了标点符号的匹配,所有的THROW最后都不要加入标点符号,所有Log最后都要加入标点符号 */
// 注意，不要这样使用：Log(cppLog, logLevel, str.c_str());即字符串默认作为格式的情况，如果字符串里有格式符，则会core掉
#ifdef USE_CPP_LOG_MACRO
#define Log(cppLog, logLevel, format, ...) if(cppLog != NULL && (cppLog)->mLogLevel <= (logLevel)){(cppLog)->LogMsg(std::string("[") + CppTime::GetUTimeStr() + "]["+CppString::ReplaceStr(#logLevel,"CppLog::")+"]" + FILE_LOCATION + "|" + CppString::GetArgs((format), ##__VA_ARGS__));}
#define LOG_THROW(cppLog, logLevel, format, ...) {Log(cppLog, logLevel, format, ##__VA_ARGS__);throw;}

// 如果定义了CPP_LOG_INSTANCE指定CppLog实例,可以使用下面的简易接口,方法是在使用前定义
#ifndef CPP_LOG_INSTANCE
#define CPP_LOG_INSTANCE &cppLog
#endif

/* 这些是传入cppLog实例的接口 */
// 普通日志
#define TRACE_LOGI(cppLog, format, ...) Log((cppLog), CppLog::TRACE, format, ##__VA_ARGS__)
#define DEBUG_LOGI(cppLog, format, ...) Log((cppLog), CppLog::DEBUG, format, ##__VA_ARGS__)
#define INFOR_LOGI(cppLog, format, ...) Log((cppLog), CppLog::INFOR, format, ##__VA_ARGS__)
#define WARNN_LOGI(cppLog, format, ...) Log((cppLog), CppLog::WARNN, format, ##__VA_ARGS__)
#define ERROR_LOGI(cppLog, format, ...) Log((cppLog), CppLog::ERROR, format, ##__VA_ARGS__)

// 检查并且带指定操作，用于构建下面更加复杂的日志
#define CHECK_OP_FI(cppLog, expr, ret, op, logLevel, format, ...) if (unlikely(!(expr))){Log((cppLog), logLevel, "Check [" #expr "] Failed,ret[%d]." format, ret, ##__VA_ARGS__);op;}
#define RET_OP_FI(cppLog, ret, op, logLevel, format, ...) if (unlikely(ret)){Log((cppLog), logLevel, "ret[%d]." format, ret, ##__VA_ARGS__);op;}
#define CHECK_OP_I(cppLog, expr, ret, op, logLevel) CHECK_OP_FI((cppLog), expr, ret, op, logLevel,"")
#define RET_OP_I(cppLog, ret, op, logLevel) RET_OP_FI((cppLog), ret, op, logLevel, "")

// 检查，不符合条件则return
#define CHECK_RETURN_FI(cppLog, expr, ret, logLevel, format, ...) CHECK_OP_FI((cppLog), expr, ret, return ret, logLevel, format, ##__VA_ARGS__)
#define RET_RETURN_FI(cppLog, ret, logLevel, format, ...) RET_OP_FI((cppLog), ret, return ret, logLevel, format, ##__VA_ARGS__)
#define CHECK_RETURN_I(cppLog, expr, ret, logLevel) CHECK_RETURN_FI((cppLog), expr, ret, logLevel, "")
#define RET_RETURN_I(cppLog, ret, logLevel) RET_RETURN_FI((cppLog), ret, logLevel, "")

#define WARNN_CHECK_RETURN_FI(cppLog, expr, ret, format, ...) CHECK_RETURN_FI((cppLog), expr, ret, CppLog::WARNN, format, ...)
#define WARNN_RET_RETURN_FI(cppLog, ret, format, ...) RET_RETURN_FI((cppLog), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define WARNN_CHECK_RETURN_I(cppLog, expr, ret) CHECK_RETURN_I((cppLog), expr, ret, CppLog::WARNN)
#define WARNN_RET_RETURN_I(cppLog, ret) RET_RETURN_I((cppLog), ret, CppLog::WARNN)

#define ERROR_CHECK_RETURN_FI(cppLog, expr, ret, format, ...) CHECK_RETURN_FI((cppLog), expr, ret, CppLog::WARNN, format, ...)
#define ERROR_RET_RETURN_FI(cppLog, ret, format, ...) RET_RETURN_FI((cppLog), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define ERROR_CHECK_RETURN_I(cppLog, expr, ret) CHECK_RETURN_I((cppLog), expr, ret, CppLog::WARNN)
#define ERROR_RET_RETURN_I(cppLog, ret) RET_RETURN_I((cppLog), ret, CppLog::WARNN)

// 检查，不符合条件则return void
#define CHECK_RETURN_VOID_FI(cppLog, expr, ret, logLevel, format, ...) CHECK_OP_FI((cppLog), expr, ret, return, logLevel, format, ##__VA_ARGS__)
#define RET_RETURN_VOID_FI(cppLog, ret, logLevel, format, ...) RET_OP_FI((cppLog), ret, return, logLevel, format, ##__VA_ARGS__)
#define CHECK_RETURN_VOID_I(cppLog, expr, ret, logLevel) CHECK_RETURN_VOID_FI((cppLog), expr, ret, logLevel, "")
#define RET_RETURN_VOID_I(cppLog, ret, logLevel) RET_RETURN_VOID_FI((cppLog), ret, logLevel, "")

#define WARNN_CHECK_RETURN_VOID_FI(cppLog, expr, ret, format, ...) CHECK_RETURN_VOID_FI((cppLog), expr, ret, CppLog::WARNN, format, ...)
#define WARNN_RET_RETURN_VOID_FI(cppLog, ret, format, ...) RET_RETURN_VOID_FI((cppLog), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define WARNN_CHECK_RETURN_VOID_I(cppLog, expr, ret) CHECK_RETURN_VOID_I((cppLog), expr, ret, CppLog::WARNN)
#define WARNN_RET_RETURN_VOID_I(cppLog, ret) RET_RETURN_VOID_I((cppLog), ret, CppLog::WARNN)

#define ERROR_CHECK_RETURN_VOID_FI(cppLog, expr, ret, format, ...) CHECK_RETURN_VOID_FI((cppLog), expr, ret, CppLog::WARNN, format, ...)
#define ERROR_RET_RETURN_VOID_FI(cppLog, ret, format, ...) RET_RETURN_VOID_FI((cppLog), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define ERROR_CHECK_RETURN_VOID_I(cppLog, expr, ret) CHECK_RETURN_VOID_I((cppLog), expr, ret, CppLog::WARNN)
#define ERROR_RET_RETURN_VOID_I(cppLog, ret) RET_RETURN_VOID_I((cppLog), ret, CppLog::WARNN)

// 检查，不符合条件则break
#define CHECK_BREAK_FI(cppLog, expr, ret, logLevel, format, ...) CHECK_OP_FI((cppLog), expr, ret, break, logLevel, format, ##__VA_ARGS__)
#define RET_BREAK_FI(cppLog, ret, logLevel, format, ...) RET_OP_FI((cppLog), ret, break, logLevel, format, ##__VA_ARGS__)
#define CHECK_BREAK_I(cppLog, expr, ret, logLevel) CHECK_BREAK_FI((cppLog), expr, ret, logLevel, "")
#define RET_BREAK_I(cppLog, ret, logLevel) RET_BREAK_FI((cppLog), ret, logLevel, "")

#define WARNN_CHECK_BREAK_FI(cppLog, expr, ret, format, ...) CHECK_BREAK_FI((cppLog), expr, ret, CppLog::WARNN, format, ...)
#define WARNN_RET_BREAK_FI(cppLog, ret, format, ...) RET_BREAK_FI((cppLog), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define WARNN_CHECK_BREAK_I(cppLog, expr, ret) CHECK_BREAK_I((cppLog), expr, ret, CppLog::WARNN)
#define WARNN_RET_BREAK_I(cppLog, ret) RET_BREAK_I((cppLog), ret, CppLog::WARNN)

#define ERROR_CHECK_BREAK_FI(cppLog, expr, ret, format, ...) CHECK_BREAK_FI((cppLog), expr, ret, CppLog::WARNN, format, ...)
#define ERROR_RET_BREAK_FI(cppLog, ret, format, ...) RET_BREAK_FI((cppLog), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define ERROR_CHECK_BREAK_I(cppLog, expr, ret) CHECK_BREAK_I((cppLog), expr, ret, CppLog::WARNN)
#define ERROR_RET_BREAK_I(cppLog, ret) RET_BREAK_I((cppLog), ret, CppLog::WARNN)

// 检查，不符合条件则contine
#define CHECK_CONTINUE_FI(cppLog, expr, logLevel, format, ...) CHECK_OP_FI((cppLog), expr, 0, continue, logLevel, format, ##__VA_ARGS__)
#define RET_CONTINUE_FI(cppLog, ret, logLevel, format, ...) RET_OP_FI((cppLog), ret, continue, logLevel, format, ##__VA_ARGS__)
#define CHECK_CONTINUE_I(cppLog, expr, logLevel) CHECK_CONTINUE_FI((cppLog), expr, logLevel, "")
#define RET_CONTINUE_I(cppLog, ret, logLevel) RET_CONTINUE_FI((cppLog), ret, logLevel, "")

#define WARNN_CHECK_CONTINUE_FI(cppLog, expr, format, ...) CHECK_CONTINUE_FI((cppLog), expr, 0, CppLog::WARNN, format, ...)
#define WARNN_RET_CONTINUE_FI(cppLog, ret, format, ...) RET_CONTINUE_FI((cppLog), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define WARNN_CHECK_CONTINUE_I(cppLog, expr) CHECK_CONTINUE_I((cppLog), expr, 0, CppLog::WARNN)
#define WARNN_RET_CONTINUE_I(cppLog, ret) RET_CONTINUE_I((cppLog), ret, CppLog::WARNN)

#define ERROR_CHECK_CONTINUE_FI(cppLog, expr, format, ...) CHECK_CONTINUE_FI((cppLog), expr, 0, CppLog::WARNN, format, ...)
#define ERROR_RET_CONTINUE_FI(cppLog, ret, format, ...) RET_CONTINUE_FI((cppLog), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define ERROR_CHECK_CONTINUE_I(cppLog, expr) CHECK_CONTINUE_I((cppLog), expr, 0, CppLog::WARNN)
#define ERROR_RET_CONTINUE_I(cppLog, ret) RET_CONTINUE_I((cppLog), ret, CppLog::WARNN)

/* 使用CPP_LOG_INSTANCE的版本 */
// 普通日志
#define TRACE_LOG(format, ...) Log((CPP_LOG_INSTANCE), CppLog::TRACE, format, ##__VA_ARGS__)
#define DEBUG_LOG(format, ...) Log((CPP_LOG_INSTANCE), CppLog::DEBUG, format, ##__VA_ARGS__)
#define INFOR_LOG(format, ...) Log((CPP_LOG_INSTANCE), CppLog::INFOR, format, ##__VA_ARGS__)
#define WARNN_LOG(format, ...) Log((CPP_LOG_INSTANCE), CppLog::WARNN, format, ##__VA_ARGS__)
#define ERROR_LOG(format, ...) Log((CPP_LOG_INSTANCE), CppLog::ERROR, format, ##__VA_ARGS__)

// 检查，不符合条件则return
#define CHECK_RETURN_F(expr, ret, logLevel, format, ...) CHECK_OP_FI((CPP_LOG_INSTANCE), expr, ret, return ret, logLevel, format, ##__VA_ARGS__)
#define RET_RETURN_F(ret, logLevel, format, ...) RET_OP_FI((CPP_LOG_INSTANCE), ret, return ret, logLevel, format, ##__VA_ARGS__)
#define CHECK_RETURN(expr, ret, logLevel) CHECK_RETURN_FI((CPP_LOG_INSTANCE), expr, ret, logLevel, "")
#define RET_RETURN(ret, logLevel) RET_RETURN_FI((CPP_LOG_INSTANCE), ret, logLevel, "")

#define WARNN_CHECK_RETURN_F(expr, ret, format, ...) CHECK_RETURN_FI((CPP_LOG_INSTANCE), expr, ret, CppLog::WARNN, format, ...)
#define WARNN_RET_RETURN_F(ret, format, ...) RET_RETURN_FI((CPP_LOG_INSTANCE), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define WARNN_CHECK_RETURN(expr, ret) CHECK_RETURN_I((CPP_LOG_INSTANCE), expr, ret, CppLog::WARNN)
#define WARNN_RET_RETURN(ret) RET_RETURN_I((CPP_LOG_INSTANCE), ret, CppLog::WARNN)

#define ERROR_CHECK_RETURN_F(expr, ret, format, ...) CHECK_RETURN_FI((CPP_LOG_INSTANCE), expr, ret, CppLog::WARNN, format, ...)
#define ERROR_RET_RETURN_F(ret, format, ...) RET_RETURN_FI((CPP_LOG_INSTANCE), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define ERROR_CHECK_RETURN(expr, ret) CHECK_RETURN_I((CPP_LOG_INSTANCE), expr, ret, CppLog::WARNN)
#define ERROR_RET_RETURN(ret) RET_RETURN_I((CPP_LOG_INSTANCE), ret, CppLog::WARNN)

// 检查，不符合条件则return void
#define CHECK_RETURN_VOID_F(expr, ret, logLevel, format, ...) CHECK_OP_FI((CPP_LOG_INSTANCE), expr, ret, return, logLevel, format, ##__VA_ARGS__)
#define RET_RETURN_VOID_F(ret, logLevel, format, ...) RET_OP_FI((CPP_LOG_INSTANCE), ret, return, logLevel, format, ##__VA_ARGS__)
#define CHECK_RETURN_VOID(expr, ret, logLevel) CHECK_RETURN_VOID_FI((CPP_LOG_INSTANCE), expr, ret, logLevel, "")
#define RET_RETURN_VOID(ret, logLevel) RET_RETURN_VOID_FI((CPP_LOG_INSTANCE), ret, logLevel, "")

#define WARNN_CHECK_RETURN_VOID_F(expr, ret, format, ...) CHECK_RETURN_VOID_FI((CPP_LOG_INSTANCE), expr, ret, CppLog::WARNN, format, ...)
#define WARNN_RET_RETURN_VOID_F(ret, format, ...) RET_RETURN_VOID_FI((CPP_LOG_INSTANCE), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define WARNN_CHECK_RETURN_VOID(expr, ret) CHECK_RETURN_VOID_I((CPP_LOG_INSTANCE), expr, ret, CppLog::WARNN)
#define WARNN_RET_RETURN_VOID(ret) RET_RETURN_VOID_I((CPP_LOG_INSTANCE), ret, CppLog::WARNN)

#define ERROR_CHECK_RETURN_VOID_F(expr, ret, format, ...) CHECK_RETURN_VOID_FI((CPP_LOG_INSTANCE), expr, ret, CppLog::WARNN, format, ...)
#define ERROR_RET_RETURN_VOID_F(ret, format, ...) RET_RETURN_VOID_FI((CPP_LOG_INSTANCE), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define ERROR_CHECK_RETURN_VOID(expr, ret) CHECK_RETURN_VOID_I((CPP_LOG_INSTANCE), expr, ret, CppLog::WARNN)
#define ERROR_RET_RETURN_VOID(ret) RET_RETURN_VOID_I((CPP_LOG_INSTANCE), ret, CppLog::WARNN)

// 检查，不符合条件则break
#define CHECK_BREAK_F(expr, ret, logLevel, format, ...) CHECK_OP_FI((CPP_LOG_INSTANCE), expr, ret, break, logLevel, format, ##__VA_ARGS__)
#define RET_BREAK_F(ret, logLevel, format, ...) RET_OP_FI((CPP_LOG_INSTANCE), ret, break, logLevel, format, ##__VA_ARGS__)
#define CHECK_BREAK(expr, ret, logLevel) CHECK_BREAK_FI((CPP_LOG_INSTANCE), expr, ret, logLevel, "")
#define RET_BREAK(ret, logLevel) RET_BREAK_FI((CPP_LOG_INSTANCE), ret, logLevel, "")

#define WARNN_CHECK_BREAK_F(expr, ret, format, ...) CHECK_BREAK_FI((CPP_LOG_INSTANCE), expr, ret, CppLog::WARNN, format, ...)
#define WARNN_RET_BREAK_F(ret, format, ...) RET_BREAK_FI((CPP_LOG_INSTANCE), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define WARNN_CHECK_BREAK(expr, ret) CHECK_BREAK_I((CPP_LOG_INSTANCE), expr, ret, CppLog::WARNN)
#define WARNN_RET_BREAK(ret) RET_BREAK_I((CPP_LOG_INSTANCE), ret, CppLog::WARNN)

#define ERROR_CHECK_BREAK_F(expr, ret, format, ...) CHECK_BREAK_FI((CPP_LOG_INSTANCE), expr, ret, CppLog::WARNN, format, ...)
#define ERROR_RET_BREAK_F(ret, format, ...) RET_BREAK_FI((CPP_LOG_INSTANCE), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define ERROR_CHECK_BREAK(expr, ret) CHECK_BREAK_I((CPP_LOG_INSTANCE), expr, ret, CppLog::WARNN)
#define ERROR_RET_BREAK(ret) RET_BREAK_I((CPP_LOG_INSTANCE), ret, CppLog::WARNN)

// 检查，不符合条件则contine
#define CHECK_CONTINUE_F(expr, logLevel, format, ...) CHECK_OP_FI((CPP_LOG_INSTANCE), expr, 0, continue, logLevel, format, ##__VA_ARGS__)
#define RET_CONTINUE_F(ret, logLevel, format, ...) RET_OP_FI((CPP_LOG_INSTANCE), ret, continue, logLevel, format, ##__VA_ARGS__)
#define CHECK_CONTINUE(expr, logLevel) CHECK_CONTINUE_FI((CPP_LOG_INSTANCE), expr, 0, logLevel, "")
#define RET_CONTINUE(ret, logLevel) RET_CONTINUE_FI((CPP_LOG_INSTANCE), ret, logLevel, "")

#define WARNN_CHECK_CONTINUE_F(expr, format, ...) CHECK_CONTINUE_FI((CPP_LOG_INSTANCE), expr, 0, CppLog::WARNN, format, ...)
#define WARNN_RET_CONTINUE_F(ret, format, ...) RET_CONTINUE_FI((CPP_LOG_INSTANCE), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define WARNN_CHECK_CONTINUE(expr) CHECK_CONTINUE_I((CPP_LOG_INSTANCE), expr, 0, CppLog::WARNN)
#define WARNN_RET_CONTINUE(ret) RET_CONTINUE_I((CPP_LOG_INSTANCE), ret, CppLog::WARNN)

#define ERROR_CHECK_CONTINUE_F(expr, ret, format, ...) CHECK_CONTINUE_FI((CPP_LOG_INSTANCE), expr, ret, CppLog::WARNN, format, ...)
#define ERROR_RET_CONTINUE_F(ret, format, ...) RET_CONTINUE_FI((CPP_LOG_INSTANCE), ret, CppLog::WARNN, format, ##__VA_ARGS__)
#define ERROR_CHECK_CONTINUE(expr, ret) CHECK_CONTINUE_I((CPP_LOG_INSTANCE), expr, ret, CppLog::WARNN)
#define ERROR_RET_CONTINUE(ret) RET_CONTINUE_I((CPP_LOG_INSTANCE), ret, CppLog::WARNN)
#endif

#define THROW(format, ...) throw CppException(0, FILE_LOCATION + "|" + CppString::GetArgs((format), ##__VA_ARGS__))
#define THROW_CODE(code, format, ...) throw CppException((code), FILE_LOCATION + "|" + CppString::GetArgs((format), ##__VA_ARGS__))

#define CHECK_THROW_F(expr, format, ...) if (unlikely(!(expr)))THROW("Check [" #expr "] Failed." format, ##__VA_ARGS__)
#define CHECK_THROW(expr) CHECK_THROW_F(expr, "")
#define RET_THROW_F(ret, format, ...) if (unlikely(ret)){THROW("ret[%d]." format, ret, ##__VA_ARGS__);}
#define RET_THROW(ret) RET_THROW_F(ret, "")

class CppException
{
public:
    int32_t Code;
    std::string Msg;

    CppException(int32_t code = 0, const std::string &msg = "") : Code(code), Msg(msg)
    {
    }

    const std::string ToString(bool showZeroCode = false) const
    {
        if (showZeroCode || Code != 0)
        {
            return CppString::GetArgs("(%d)%s", Code, Msg.c_str());
        }

        return Msg;
    }

    friend ostream &operator<<(ostream &os, const CppException &right)
    {
        return os << right.ToString();
    }
};

class CppLog
{
public:
    enum LOG_LEVEL
    {
        TRACE,
        DEBUG,
        INFOR,
        WARNN,
        ERROR
    };

    CppLog(const std::string &logFile = "", LOG_LEVEL logLevel = TRACE, uint32_t maxFileSize = 0, uint32_t maxFileCount = 1);

    //************************************
    // Describe:  记录日志
    // Parameter: const std::string & msg
    // Returns:   void
    // Author:    moon
    //************************************
    void LogMsg(const std::string &msg);

    std::string mLogFile;           // 记录日志的文件
    LOG_LEVEL mLogLevel;            // 日志级别
    uint32_t mMaxFileSize;          // 日志文件最大大小,0表示不限制
    uint32_t mMaxFileCount;         // 日志文件最大数量,0表示不保存日志

    /** 获得调用栈
     *
     * @retval  string
     * @author  moontan
     */
    static string GetStackTrace();
};

#endif
