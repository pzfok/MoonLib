#include "CppCrypto.h"

#include <stdint.h>

#include <fstream>

#include <openssl/md5.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>  
#include <openssl/buffer.h>  

#include "CppString.h"

using namespace std;

string CppCrypto::Md5(const string &data, bool upCase)
{
    unsigned char md[MD5_DIGEST_LENGTH];

    MD5((unsigned char *)data.c_str(), data.size(), (unsigned char*)&md);
    return CppString::Hex2Str(string((const char *)md, MD5_DIGEST_LENGTH), upCase);
}

string CppCrypto::Md5File(const string &path)throw(CppException)
{
    unsigned char md[MD5_DIGEST_LENGTH];

    const uint32_t BUF_SIZE = 1024;
    char buf[BUF_SIZE];

    ifstream ifs(path.c_str(), ios::in);
    if (!ifs)
    {
        THROW("Open file[%s] fail", path.c_str());
    }

    MD5_CTX c;
    MD5_Init(&c);

    while (!ifs.eof())
    {
        ifs.read(buf, sizeof(buf));
        MD5_Update(&c, buf, ifs.gcount());
    }

    ifs.close();
    MD5_Final(md, &c);

    return CppString::Hex2Str((const char *)md, MD5_DIGEST_LENGTH);
}

string CppCrypto::HmacSHA256(const string &key, const string &data)
{
    // http://blog.csdn.net/yasi_xi/article/details/9066003

    unsigned int resultLen = EVP_MAX_MD_SIZE;
    string result(resultLen, 0);

    (void)HMAC(EVP_sha256(), key.data(), key.size(),
               reinterpret_cast<const unsigned char *>(data.data()), data.size(),
               reinterpret_cast<unsigned char *>(&result[0]), &resultLen);

    result.resize(resultLen);
    return result;
}

string CppCrypto::HmacSHA384(const string &key, const string &data)
{
    // http://blog.csdn.net/yasi_xi/article/details/9066003

    unsigned int resultLen = EVP_MAX_MD_SIZE;
    string result(resultLen, 0);

    (void)HMAC(EVP_sha384(), key.data(), key.size(),
               reinterpret_cast<const unsigned char *>(data.data()), data.size(),
               reinterpret_cast<unsigned char *>(&result[0]), &resultLen);

    result.resize(resultLen);
    return result;
}

string CppCrypto::Base64Encode(const string &data)
{
    // http://blog.csdn.net/yasi_xi/article/details/9040793

    BIO * bmem = NULL;
    BIO * b64 = NULL;
    BUF_MEM * bptr = NULL;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, data.data(), data.size());
    (void)BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    string result(bptr->data, bptr->length);
    BIO_free_all(b64);

    return result;
}
