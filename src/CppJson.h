#ifndef _CPP_JSON_H_
#define _CPP_JSON_H_

#include <string>

#include "rapidjson/document.h"

#include "CppLog.h"

using std::string;

class CppJson
{
public:

    //************************************
    // Describe:  将Json::Value转为可读字符串
    // Parameter: const Json::Value & jsonValue
    // Returns:   string
    // Author:    moon
    //************************************
    static string JsonToStyledStr(const rapidjson::Document &jsonValue);

    //************************************
    // Describe:  将Json::Value转为一行可读字符串
    // Parameter: const Json::Value & jsonValue
    // Returns:   string
    // Author:    moon
    //************************************
    static string JsonToOneLineStr(const rapidjson::Document &jsonValue);
};

#endif
