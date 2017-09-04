#ifndef _CPP_MYSQL_H_
#define _CPP_MYSQL_H_

#include <string>
#include <map>
#include <vector>

#include <cmysql/cmysql.h>

#include "CppString.h"
#ifndef USE_CPP_LOG_MACRO
#define USE_CPP_LOG_MACRO
#endif
#include "CppLog.h"

#define DEFINE_MYSQL_ROWS_VECTOR(name) std::vector<std::string> &name##_rows = data[#name]

class CppMysql
{
public:
    static void MysqlQuery(MYSQL &mysql, const std::string &sqlCmd, CppLog *pCppLog = NULL)
    {
        Log(pCppLog, CppLog::DEBUG, "%s", sqlCmd.c_str());

        if (mysql_query(&mysql, sqlCmd.c_str()))
        {
            THROW("mysql_query[%s]错误:%s", sqlCmd.c_str(), mysql_error(&mysql));
        }
    }

    static void MysqlQuery(CMysql &mysql, const std::string &sqlCmd, CppLog *pCppLog = NULL) throw(CppException)
    {
        Log(pCppLog, CppLog::DEBUG, "%s", sqlCmd.c_str());

        try
        {
            mysql.FreeResult();
            mysql.Query(sqlCmd.c_str());
        }
        catch (const CMysqlException &e)
        {
            THROW("CMysql执行错误[%s].", e.GetErrMsg());
        }
    }

    static void StoreResultToMap(CMysql &mysql, std::map<std::string, std::vector<std::string> > &data);

    static std::string GetMysqlResult(const std::map<std::string, std::vector<std::string> > &data,
                                      const std::string &feildName, uint32_t row);
};
#endif
