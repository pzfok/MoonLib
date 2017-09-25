#include <iostream>
#include <thread>

#include "gtest/gtest.h"

#include <CppCurl.h>
#include <CppLog.h>

static CppLog cppLog;

using namespace std;

TEST(CppCurl, GetUrlEncodeTest)
{
    //     EXPECT_EQ("123465asd", CppCurl::GetUrlEncode("123465asd"));
    //     EXPECT_EQ("%E9%97%AE", CppCurl::GetUrlEncode("问"));
}

TEST(CppCurl, ThreadTest)
{
    try
    {
        static const uint32_t THREAD_COUNT = 200;

        vector<thread> tds;
        for (uint32_t i = 0; i < THREAD_COUNT; ++i)
        {
            thread t([&]()
            {
                try
                {
                    CppCurl::Get("www.baidu.com", "",
                                 vector<string>(), "", 30);
                }
                catch (const CppException &e)
                {
                    ERROR_LOG("抛出异常:%s", e.ToString().c_str());
                }
            });
            tds.push_back(move(t));
        }

        for (auto &t : tds)
        {
            t.join();
        }
    }
    catch (const CppException &e)
    {
        ERROR_LOG("抛出异常:%s", e.ToString().c_str());
    }
}
