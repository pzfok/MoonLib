#include "CppJson.h"

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"

using namespace rapidjson;

rapidjson::Document CppJson::ParseJson(const string &json_str)
{
    rapidjson::Document d;
    d.Parse(json_str.c_str());
    return d;
}

string CppJson::JsonToStyledStr(const rapidjson::Document &jsonValue)
{
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    jsonValue.Accept(writer);
    return buffer.GetString();
}

string CppJson::JsonToOneLineStr(const rapidjson::Document &jsonValue)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    jsonValue.Accept(writer);
    return buffer.GetString();
}
