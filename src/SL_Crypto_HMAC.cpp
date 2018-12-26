#include "SL_Crypto_Hash.h"
#include "SL_Crypto_HMAC.h"

#ifdef SOCKETLITE_USE_OPENSSL
#include <openssl/evp.h>

SL_Crypto_HMAC::SL_Crypto_HMAC()
{
    HMAC_CTX_init(&ctx_);
}

SL_Crypto_HMAC::~SL_Crypto_HMAC()
{
    HMAC_CTX_cleanup(&ctx_);
}

int SL_Crypto_HMAC::init(const unsigned char *key, int len, int hash_type)
{
    const EVP_MD *evp_md;
    switch (hash_type)
    {
        case SL_Crypto_Hash::MD5:
            evp_md = EVP_md5();
            break;
        case SL_Crypto_Hash::SHA1:
            evp_md = EVP_sha1();
            break;
        case SL_Crypto_Hash::SHA256:
            evp_md = EVP_sha256();
            break;
        case SL_Crypto_Hash::SHA512:
            evp_md = EVP_sha512();
            break;
        default:
            evp_md = EVP_sha256();
            break;
    }
    return HMAC_Init_ex(&ctx_, key, len, evp_md, NULL);
}

int SL_Crypto_HMAC::update(const unsigned char *data, unsigned int len)
{
    return HMAC_Update(&ctx_, data, len);
}

int SL_Crypto_HMAC::final(unsigned char *md, unsigned int *md_len)
{
    return HMAC_Final(&ctx_, md, md_len);
}

int SL_Crypto_HMAC::final(const unsigned char *data, unsigned int len, unsigned char *md, unsigned int *md_len)
{
    int ret = HMAC_Update(&ctx_, data, len);
    if (!ret)
    {
        return -1;
    }
    return HMAC_Final(&ctx_, md, md_len);
}

unsigned char* SL_Crypto_HMAC::final(const unsigned char *key, int key_len, int hash_type, const unsigned char *data, unsigned int data_len, unsigned char *md, unsigned int *md_len)
{
    const EVP_MD *evp_md;
    switch (hash_type)
    {
        case SL_Crypto_Hash::MD5:
            evp_md = EVP_md5();
            break;
        case SL_Crypto_Hash::SHA1:
            evp_md = EVP_sha1();
            break;
        case SL_Crypto_Hash::SHA256:
            evp_md = EVP_sha256();
            break;
        case SL_Crypto_Hash::SHA512:
            evp_md = EVP_sha512();
            break;
        default:
            evp_md = EVP_sha256();
            break;
    }
    return HMAC(evp_md, key, key_len, data, data_len, md, md_len);
}

#else

SL_Crypto_HMAC::SL_Crypto_HMAC()
{
}

SL_Crypto_HMAC::~SL_Crypto_HMAC()
{
}

int SL_Crypto_HMAC::init(const unsigned char *key, int len, int hash_type)
{
    return 0;
}

int SL_Crypto_HMAC::update(const unsigned char *data, unsigned int len)
{
    return 0;
}

int SL_Crypto_HMAC::final(unsigned char *md, unsigned int *md_len)
{
    return 0;
}

int SL_Crypto_HMAC::final(const unsigned char *data, unsigned int len, unsigned char *md, unsigned int *md_len)
{
    return 0;
}

unsigned char* SL_Crypto_HMAC::final(const unsigned char *key, int key_len, int hash_type, const unsigned char *data, unsigned int data_len, unsigned char *md, unsigned int *md_len)
{
    return 0;
}

#endif

