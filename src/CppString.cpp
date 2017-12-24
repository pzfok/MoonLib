#include "CppString.h"

#include <iconv.h>
#include <alloca.h>

#include <fstream>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <numeric>
#include <algorithm>

using namespace std;

string CppString::Reverse(const string &srcString)
{
    size_t len = srcString.length();
    string outString;

    for (size_t i = 0; i < len; ++i)
    {
        outString += srcString[len - i - 1];
    }

    return outString;
}

string CppString::ReplaceStr(string str, const string &oldValue, const string &newValue)
{
    string::size_type pos(0);

    while (true)
    {
        //查找词
        if ((pos = str.find(oldValue, pos)) != string::npos)
        {
            str.replace(pos, oldValue.length(), newValue);
            pos += newValue.length() - oldValue.length() + 1;
        }
        else
        {
            break;
        }
    }

    return str;
}

const string CppString::ToPrecisionDouble(uint32_t precision, double value)
{
    string patten = "%." + to_string(precision) + "f";
    return CppString::GetArgs(patten.c_str(), value);
}

void CppString::SplitStr(string str, const string &splitStr, vector<string> &result, bool removeEmptyElm, size_t maxCount)
{
    SplitStr(str, vector<string>(1, splitStr), result, removeEmptyElm, maxCount);
}

void CppString::SplitStr(string str, const vector<string> &splitStr, vector<string> &result, bool removeEmptyElm, size_t maxCount)
{
    result.clear();
    size_t currCount = 0;          // 当前已获得段数

    // 从所有分割字符串中查找最小的索引
    size_t index = string::npos;
    size_t splitLen = 0;
    size_t currIndex;
    for (vector<string>::const_iterator it = splitStr.begin(); it != splitStr.end(); ++it)
    {
        if (it->length() == 0)
        {
            continue;
        }

        currIndex = str.find(*it);
        if (currIndex != string::npos && currIndex < index)
        {
            index = currIndex;
            splitLen = it->length();
        }
    }

    while (index != string::npos)
    {
        if (index != 0 || !removeEmptyElm)
        {
            // 将找到的字符串放入结果中
            ++currCount;
            if (maxCount > 0 && currCount >= maxCount)
            {
                break;
            }
            result.push_back(str.substr(0, index));
        }

        // 将之前的字符和分割符都删除
        str.erase(str.begin(), str.begin() + index + splitLen);

        // 继续查找下一个
        index = string::npos;
        for (vector<string>::const_iterator it = splitStr.begin(); it != splitStr.end(); ++it)
        {
            if (it->length() == 0)
            {
                continue;
            }

            currIndex = str.find(*it);
            if (currIndex != string::npos && currIndex < index)
            {
                index = currIndex;
                splitLen = it->length();
            }
        }
    }

    // 把剩下的放进去
    if (str.length() > 0 || !removeEmptyElm)
    {
        result.push_back(str);
    }
}

string CppString::RemoveAngle(string str, const char leftChar, const char rightChar)
{
    int startIndex = 0; // leftChar的位置
    int endIndex = 0;   // rightChar的位置

    while (true)
    {
        startIndex = str.find(leftChar);
        endIndex = str.find(rightChar, startIndex + 1);
        if (startIndex < 0 || endIndex < 0 || startIndex >= endIndex)
        {
            break;
        }

        str.erase(startIndex, endIndex - startIndex + 1);
    }

    return str;
}

string CppString::GetList(const string str[], size_t n, const string &splitCh)
{
    string result;

    if (n > 0)
    {
        result += str[0];
    }

    for (size_t i = 1; i < n; ++i)
    {
        result += splitCh + str[i];
    }

    return result;
}

string CppString::GetList(const vector<string> &str, const string &splitCh)
{
    string result;

    size_t n = str.size();

    if (n > 0)
    {
        result += str[0];
    }

    for (size_t i = 1; i < n; ++i)
    {
        result += splitCh + str[i];
    }

    return result;
}

string CppString::GetList(const set<string> &str, const string &splitCh)
{
    string result;

    size_t n = str.size();

    set<string>::const_iterator it = str.begin();

    if (n > 0)
    {
        result += *it++;
    }

    for (; it != str.end(); ++it)
    {
        result += splitCh + *it;
    }

    return result;
}

string CppString::Unique(const string &str, const string &splitCh, bool removeEmptyElm)
{
    vector<string> resultVec;
    CppString::SplitStr(str, splitCh, resultVec, removeEmptyElm);

    set<string> resultSet;
    resultSet.insert(resultVec.begin(), resultVec.end());

    return GetList(resultSet, splitCh);
}

string CppString::TrimLeft(string str, const string &trimStr, int32_t times /*= 1*/)
{
    while (times == -1 || times-- > 0)
    {
        if (!trimStr.empty() && str.find(trimStr) == 0)
        {
            str = str.substr(trimStr.length());
        }
        else
        {
            break;
        }
    }

    return str;
}

static const vector<char> WHITESPACES{ 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x20, 0 };

string CppString::TrimLeft(string str, const vector<string> &trimStrs, int32_t times /*= 1*/)
{
    bool notFound;

    while (times == -1 || times-- > 0)
    {
        notFound = true;

        for (vector<string>::const_iterator it = trimStrs.begin(); it != trimStrs.end(); ++it)
        {
            if (!it->empty() && str.find(*it) == 0)
            {
                notFound = false;
                str = str.substr(it->length());
                break;
            }
        }

        if (notFound)
        {
            break;
        }
    }

    return str;
}

string CppString::TrimLeft(const string &str, int32_t times /*= 1*/)
{
    uint32_t i = 0;
    for (; i < str.size() && (times == -1 || static_cast<int32_t>(i) < times); ++i)
    {
        if (find(WHITESPACES.begin(), WHITESPACES.end(), str[i]) == WHITESPACES.end())
        {
            break;
        }
    }

    return str.substr(i);
}

string CppString::TrimRight(string str, const string &trimStr, int32_t times /*= 1*/)
{
    string::size_type pos;

    while (times == -1 || times-- > 0)
    {
        if (trimStr.empty())
        {
            break;
        }

        pos = str.rfind(trimStr);
        if (pos != string::npos && pos + trimStr.length() == str.length())
        {
            str = str.substr(0, pos);
        }
        else
        {
            break;
        }
    }

    return str;
}

string CppString::TrimRight(string str, const vector<string> &trimStrs, int32_t times /*= 1*/)
{
    string::size_type pos;
    bool notFound;

    while (times == -1 || times-- > 0)
    {
        notFound = true;

        for (vector<string>::const_iterator it = trimStrs.begin(); it != trimStrs.end(); ++it)
        {
            if (it->empty())
            {
                continue;
            }

            pos = str.rfind(*it);
            if (pos != string::npos && pos + it->length() == str.length())
            {
                notFound = false;
                str = str.substr(0, pos);
                break;
            }
        }

        if (notFound)
        {
            break;
        }
    }

    return str;
}

string CppString::TrimRight(const string &str, int32_t times /*= 1*/)
{
    int32_t i = str.size() - 1;
    for (; i >= 0 && (times == -1 || static_cast<int32_t>(str.size() - 1 - i) < times); --i)
    {
        if (find(WHITESPACES.begin(), WHITESPACES.end(), str[i]) == WHITESPACES.end())
        {
            break;
        }
    }

    return str.substr(0, i + 1);
}

string CppString::Trim(string str, const string &trimStr, int32_t times /*= 1*/)
{
    return TrimRight(TrimLeft(str, trimStr, times), trimStr, times);
}

string CppString::Trim(string str, const vector<string> &trimStrs, int32_t times /*= 1*/)
{
    return TrimRight(TrimLeft(str, trimStrs, times), trimStrs, times);
}

string CppString::Trim(string str, int32_t times /*= 1*/)
{
    return TrimRight(TrimLeft(str, times), times);
}

string CppString::ToLower(string str)
{
    for (string::iterator it = str.begin(); it != str.end(); ++it)
    {
        *it = tolower(*it);
    }

    return str;
}

string CppString::ToUpper(string str)
{
    for (string::iterator it = str.begin(); it != str.end(); ++it)
    {
        *it = toupper(*it);
    }

    return str;
}

uint32_t CppString::SubstrCount(string str, const string &subStr)
{
    uint32_t count = 0;
    string::size_type pos = 0;

    if (subStr.length() == 0 || str.length() == 0)
    {
        return 0;
    }

    while ((pos = str.find(subStr)) != string::npos)
    {
        str.erase(0, pos + 1);
        ++count;
    }

    return count;
}

string CppString::GetArgs(const char *format, ...)
{
    const size_t BUF_SIZE = 409600;

    va_list list;
    va_start(list, format);

    char buf[BUF_SIZE];
    vsnprintf(buf, sizeof(buf), format, list);
    va_end(list);

    return buf;
}

char CppString::Hex2Char(uint8_t c, bool upCase)
{
    c &= 0xf;
    if (c < 10)
    {
        return c + '0';
    }

    return c + (upCase ? 'A' : 'a') - 10;
}

string CppString::Hex2Str(const string &str, bool upCase /*= false*/)
{
    string::size_type len = str.length();
    string result;
    result.reserve(len << 1);

    for (string::size_type i = 0; i < len; ++i)
    {
        result += Hex2Char(str[i] >> 4, upCase);
        result += Hex2Char(str[i], upCase);
    }

    return result;
}

int32_t CodeConv(iconv_t &cd, const string &src, string &dst)
{
    char *pSource = const_cast<char *>(src.data());
    size_t srcLen = src.length();
    size_t dstLen = srcLen * 1.5 + 1;
    size_t oldDstLen = dstLen;

    char *pDst = reinterpret_cast<char *>(alloca(dstLen));
    if (pDst == NULL)
    {
        return -3;
    }

    char *pOldPtr = pDst;            // 因为使用iconv后，地址会改变，所以要记录原始地址
    char **ppDest = reinterpret_cast<char **>(&pDst);

    int32_t ret = iconv(cd, &pSource, &srcLen, ppDest, &dstLen);
    iconv_close(cd);
    if (ret == 0)
    {
        dst.assign(pOldPtr, oldDstLen - dstLen);
    }
    else if (ret == -1)
    {
        return errno == EILSEQ;
    }

    return ret;
}

int32_t CppString::Utf8ToGb2312(const string &utf8Src, string &gb2312Dst)
{
    iconv_t cd;
    if ((cd = iconv_open("gb2312", "utf-8")) == 0)
    {
        return -2;
    }

    return CodeConv(cd, utf8Src, gb2312Dst);
}

int32_t CppString::Gb2312ToUtf8(const string &gb2312Src, string &utf8Dst)
{
    iconv_t cd;
    if ((cd = iconv_open("utf-8", "gb2312")) == 0)
    {
        return -2;
    }

    return CodeConv(cd, gb2312Src, utf8Dst);
}

uint32_t CppString::EditDistance(const string &str1, const string &str2)
{
    // 计算方法：
    // http://www.cnblogs.com/ivanyb/archive/2011/11/25/2263356.html
    // https://baike.baidu.com/item/%E7%BC%96%E8%BE%91%E8%B7%9D%E7%A6%BB

    if (str1.empty())
    {
        return str2.length();
    }

    if (str2.empty())
    {
        return str1.length();
    }
    /*
    // 计算矩阵,str1是左侧竖着排列，str2是上方横着排列
    vector<vector<uint32_t>> mat(str1.length() + 1, vector<uint32_t>(str2.length() + 1, 0));

    // 初始化矩阵
    iota(mat[0].begin(), mat[0].end(), 0);
    for (uint32_t r = 1; r <= str1.length(); ++r)
    {
        *mat[r].begin() = r;
    }

    // 开始填充矩阵剩下的部分
    uint32_t min_value;
    for (uint32_t c = 1; c <= str2.length(); ++c)
    {
        for (uint32_t r = 1; r <= str1.length(); ++r)
        {
            min_value = mat[r - 1][c - 1] + ((str1[r - 1] == str2[c - 1]) ? 0 : 1);
            if (min_value > mat[r - 1][c] + 1)
            {
                min_value = mat[r - 1][c] + 1;
            }

            if (min_value > mat[r][c - 1] + 1)
            {
                min_value = mat[r][c - 1] + 1;
            }

            mat[r][c] = min_value;
        }
    }

    //     for (uint32_t r = 0; r <= str1.length(); ++r)
    //     {
    //         for (uint32_t c = 0; c <= str2.length(); ++c)
    //         {
    //             cout << mat[r][c] << " ";
    //         }
    //         cout << endl;
    //     }

    return mat[str1.length()][str2.length()];
    */

    // 这里只记录矩阵的一列，因为一列用过之后就再也没用了，并且和百度百科里不同的是，值从1开始，因为总是需要+1
    // 左上角的，如果行列字符相同，就-1，不同就不变
    vector<uint32_t> mat_col(str1.length() + 1, 0);

    // 初始化矩阵
    iota(mat_col.begin(), mat_col.end(), 1);

    uint32_t min_value;
    uint32_t last_left_up_value;        // 上次的左上方的值
    for (uint32_t c = 1; c <= str2.length(); ++c)
    {
        // 记录并且更新这一列第一行的值
        last_left_up_value = mat_col[0]++;

        for (uint32_t r = 1; r <= str1.length(); ++r)
        {
            // 取左上方的值
            min_value = last_left_up_value - ((str1[r - 1] == str2[c - 1]) ? 1 : 0);

            // 取上方的值
            if (min_value > mat_col[r - 1])
            {
                min_value = mat_col[r - 1];
            }

            // 取左方的值
            if (min_value > mat_col[r])
            {
                min_value = mat_col[r];
            }

            // 存储到当前位置，值+1
            last_left_up_value = mat_col[r];
            mat_col[r] = min_value + 1;
        }
    }

    // 因为所有存储的值都+1了，所以实际值需要-1
    return *mat_col.rbegin() - 1;
}
