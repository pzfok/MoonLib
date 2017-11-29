#include <iostream>
#include <string>

#include "gtest/gtest.h"

#include <CppCurl.h>
#include <CppRegex.h>

using namespace std;

TEST(CppTest, DISABLED_DTSMusic)
{
    // 列表地址：http://blog.sina.com.cn/s/articlelist_1197845451_8_1.html
    // http://blog.sina.com.cn/s/articlelist_2477486401_1_1.html

    CppRegex titleReg("<title>.*?</title>");
    CppRegex listReg("http://blog.sina.com.cn/s/blog_.*?html");
    for (uint32_t page = 1; page <= 25; ++page)
    {
        //string html = CppCurl::Get(CppString::GetArgs("http://blog.sina.com.cn/s/articlelist_1197845451_8_%d.html", page));
        string html = CppCurl::Get(CppString::GetArgs("http://blog.sina.com.cn/s/articlelist_2477486401_1_%d.html", page));
        vector<string> result;
        listReg.Matches(html, result);
        for (auto result_it = result.begin(); result_it != result.end(); ++result_it)
        {
            html = CppCurl::Get(*result_it);
            string title = CppString::ReplaceStr(CppString::ReplaceStr(titleReg.Match(html), "_jiangkuaihe_新浪博客</title>"), "<title>");
            title = CppString::ReplaceStr(title, "&nbsp;", " ");
            cout << title << endl << *result_it << endl << endl;
        }
    }
}

TEST(CppTest, VTable)
{
    class Base
    {
    public:
        virtual string f()
        {
            return "Base::f";
        }
        virtual string g()
        {
            return "Base::g";
        }
        virtual string h()
        {
            return "Base::h";
        }
    };

    typedef string(*Fun)(void);
    Base b;
    Fun pFun = NULL;

    //     cout << "虚函数表地址：" << (int64_t*)(&b) << endl;
    //     cout << "虚函数表 ― 第1个函数地址：" << (int64_t*)*(int64_t*)(&b) << endl;
    //     cout << "虚函数表 ― 第2个函数地址：" << (int64_t*)*(int64_t*)(&b) + 1 << endl;
    //     cout << "虚函数表 ― 第3个函数地址：" << (int64_t*)*(int64_t*)(&b) + 2 << endl;

    // Invoke the first virtual function
    pFun = reinterpret_cast<Fun>(*(reinterpret_cast<int64_t*>(*reinterpret_cast<int64_t*>(&b))));
    EXPECT_EQ("Base::f", pFun());
    pFun = reinterpret_cast<Fun>(*(reinterpret_cast<int64_t*>(*reinterpret_cast<int64_t*>(&b)) + 1));
    EXPECT_EQ("Base::g", pFun());
    pFun = reinterpret_cast<Fun>(*(reinterpret_cast<int64_t*>(*reinterpret_cast<int64_t*>(&b)) + 2));
    EXPECT_EQ("Base::h", pFun());
}

TEST(CppTest, TempTest)
{
    uint32_t a = 10;
    uint32_t b = 42;
    cout << a - b << endl;
}

TEST(CppTest, defaultMacro)
{
    cout << __FILE__ << endl;           // src/CppTest.cpp
    cout << __LINE__ << endl;           // 81
    cout << __DATE__ << endl;           // Jan 16 2017
    cout << __TIME__ << endl;           // 11:26:35
    cout << __TIMESTAMP__ << endl;      // Mon Jan 16 11:26:34 2017
    cout << __FUNCTION__ << endl;       // TestBody
}

void defaultFunc(int a, int b, int c = 1);

// 可以在现有的默认参数定义中添加默认参数
void defaultFunc(int a, int b = 2, int c);

void defaultFunc(int a, int b, int c)
{
    static_cast<void>(a);
    static_cast<void>(b);
    static_cast<void>(c);
}

TEST(CppTest, defaultParam)
{
    defaultFunc(1);
    defaultFunc(1, 2);
    defaultFunc(1, 2, 3);
}

string Taobaoke(const string &good_id)
{
    // curl 'http://pub.alimama.com/common/code/getAuctionCode.json?auctionid=557686932596&adzoneid=15058629&siteid=4744205&scenes=1&t=1508908087960' 
    // -H 'Cookie: cookie2=5dd8d7d07fb859443352ea9c5d5ed26c; t=cc58a95ec533e47b29c5bc1fc7696e86; v=0; _tb_token_=wG5a2Is7H5Xq; account-path-guide-s1=true; 19068717_yxjh-filter-1=true; tlut=UoW%2Bu4qj4yhTPQ%3D%3D; l=AlVViKFo5MsTLHX-fRRYXxJj5VsPOAlk; BAIDU_SSP_lcr=http://www.ali213.net/news/html/2017-3/287747.html; alimamapwag=TW96aWxsYS81LjAgKFdpbmRvd3MgTlQgMTAuMDsgV2luNjQ7IHg2NCkgQXBwbGVXZWJLaXQvNTM3LjM2IChLSFRNTCwgbGlrZSBHZWNrbykgQ2hyb21lLzU5LjAuMzA3MS4xMDQgU2FmYXJpLzUzNy4zNg%3D%3D; cookie32=434cdfdb0b70062476a9bc9f9c9ee61f; alimamapw=QxsVH09tCFZRAFMCVQVUUFQPCVACAwUHC1QGBwdSVQMCBlBcVQA%3D; cookie31=MTkwNjg3MTcsc3l0enosdHp6MTk5MDA1MTNAZ21haWwuY29tLFRC; login=WqG3DMC9VAQiUQ%3D%3D; cna=qbVzEfGZcUwCAQ4RFiFx4f2t; apush73d0a9a50074cda3cefc2b0534fc9ba2=%7B%22ts%22%3A1508908087226%2C%22heir%22%3A1508908043482%2C%22parentId%22%3A1508907131007%7D; isg=AgYG5wZeo-FQLHaYpUuwLIlCV_yCRz4VVAoEUPAqoCkJ868NXPZQM2vFvRjF' 
    // -H 'Accept-Encoding: gzip, deflate' 
    // -H 'Accept-Language: en-US,en;q=0.8,zh-CN;q=0.6,zh;q=0.4' 
    // -H 'User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.104 Safari/537.36'
    // -H 'Accept: application/json, text/javascript, */*; q=0.01'
    // -H 'Cache-Control: no-cache'
    // -H 'X-Requested-With: XMLHttpRequest' -H 'Proxy-Connection: keep-alive' 
    
    // string cookies = NewsDb::Instance().GetKV("taobaoke_cookies");
    string url = CppString::GetArgs("http://pub.alimama.com/common/code/getAuctionCode.json?auctionid=%s&adzoneid=15058629&siteid=4744205&scenes=1&t=%lu",good_id.c_str(),CppTime::GetUTime()/1000);
    vector<string> headers={
        "Accept-Encoding: gzip, deflate",
        "Accept-Language: en-US,en;q=0.8,zh-CN;q=0.6,zh;q=0.4",
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.104 Safari/537.36",
        "Accept: application/json, text/javascript, */*; q=0.01",
        "Cache-Control: no-cache",
        "X-Requested-With: XMLHttpRequest' -H 'Proxy-Connection: keep-alive",
        "Cookie:thw=cn; mt=ci=35_1; uc1=cookie16=W5iHLLyFPlMGbLDwA%2BdvAGZqLg%3D%3D&cookie21=VT5L2FSpccLuJBrWTbuV&cookie15=UtASsssmOIJ0bQ%3D%3D&existShop=false&pas=0&cookie14=UoTcBzbNGZAvfQ%3D%3D&tag=10&lng=zh_CN; isg=AhUVQJLd4KU4I8RZPFBN80IlKBjPEskkpslbT5e60Qzb7jXgX2LZ9CMsaMgn; cna=Oil4Eps3yn0CAQ5/+4KeLMNH; _mw_us_time_=1508981585124; cookie17=Vvzwt0CEJpo%3D; _nk_=sytzz; _l_g_=Ug%3D%3D; tg=0; _cc_=W5iHLLyFfA%3D%3D; t=cced3a21cb99f075b96d01d37c2fca20; skt=d5109431719bc7a0; unb=52306844; cookie1=BqJl%2FOBgEi0yuxImMNxU0PxI%2FS47GWIw%2FWDRKaPIJms%3D; sg=z4e; cookie2=19db952c3a445b193c3fdead6ccad871; tracknick=sytzz; lgc=sytzz; uss=VADfAHStSFeMnPXtdJ5FqSsa1vBZuBxFhXMZEcAma8DRCfCnOL3nJIv7; existShop=MTUwODk4MTU4Mw%3D%3D; uc3=sg2=AiOcmiTDewTLX5vKlm6b%2BId5cfGzBdGI54%2Fhd9%2FIM%2Bk%3D&nk2=EEcRYV4%3D&id2=Vvzwt0CEJpo%3D&vt3=F8dBzLEyH17JTk0ZPRQ%3D&lg2=WqG3DMC9VAQiUQ%3D%3D; _tb_token_=37de5eae56ea3; v=0"
    };
    
    string result = CppCurl::Get(url,"/tmp/taobao",headers);
    printf("%s\n",result.c_str());
    return result;
}

TEST(CppTest,test)
{
    Taobaoke("558200316749");
}
