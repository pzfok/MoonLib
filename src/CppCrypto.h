#ifndef _CPP_CRYPTO_H_
#define _CPP_CRYPTO_H_

#include <string>

#include "CppLog.h"

using std::string;

class CppCrypto
{
public:

    /** 计算MD5值
     *
     * @param 	const string & data     原数据
     * @param 	bool upCase             结果是大写还是小写，默认是大写
     * @retval 	string                  返回MD5的可读字符串格式
     * @author 	moon
     */
    static string Md5(const string &data, bool upCase = true);

    //************************************
    // Describe:  计算文件的MD5值
    // Parameter: const string & path   文件路径
    // Returns:   string                返回MD5的可读字符串格式
    // Author:    moontan
    //************************************
    static string Md5File(const string &path)throw(CppException);
};

#endif
