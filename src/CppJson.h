#pragma once

#include <string>

#include <rapidjson/document.h>

#include "CppLog.h"

using std::string;

class CppJson
{
public:

    /** 解析Json
     *
     * @param   const string & json_str
     * @retval  rapidjson
     * @author  moon
     */
    static rapidjson::Document ParseJson(const string &json_str);

    /** 将rapidjson::Document转为可读字符串
     *
     * @param   const rapidjson::Document & jsonValue
     * @retval  string
     * @author  moon
     */
    static string JsonToStyledStr(const rapidjson::Document &jsonValue);

    /** 将rapidjson::Document转为一行可读字符串
     *
     * @param   const rapidjson::Document & jsonValue
     * @retval  string
     * @author  moon
     */
    static string JsonToOneLineStr(const rapidjson::Document &jsonValue);
};
