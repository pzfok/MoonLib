#pragma once

#include <string>

class CppCompress
{
public:

    // gzip相关代码来源于：https://github.com/chafey/GZipCodec

    // GZip Decompression
    //
    // @param   const std::string & compressedData      压缩后的数据
    // @param   std::string & data                      传出解压后的数据
    // @retval  bool                                    成功返回true，失败返回false
    // @author  moon    
    static bool GzipUncompress(const std::string& compressedData, std::string& data);

    // GZip Compression
    //
    // @param   const std::string & data                原始数据
    // @param   std::string & compressedData            压缩后的数据
    // @param   int level                               压缩级别 -1 = default, 0 = no compression, 1= worst/fastest compression, 9 = best/slowest compression
    // @retval  bool                                    成功返回true，失败返回false
    // @author  moon    
    static bool GzipCompress(const std::string& data, std::string& compressedData, int level = -1);
};
