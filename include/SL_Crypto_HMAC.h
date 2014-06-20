#ifndef SOCKETLITE_CRYPTO_HMAC_H
#define SOCKETLITE_CRYPTO_HMAC_H
#include "SL_Crypto_Cipher.h"

#ifdef SOCKETLITE_USE_OPENSSL
#include <openssl/hmac.h>

class SL_Crypto_HMAC
{
public:    
    SL_Crypto_HMAC();
    virtual ~SL_Crypto_HMAC();

    int init(const unsigned char *key, int len, int hash_type);
    int update(const unsigned char *data, unsigned int len);
    int final(unsigned char *md, unsigned int *md_len);
    int final(const unsigned char *data, unsigned int len, unsigned char *md, unsigned int *md_len);

    static unsigned char* final(const unsigned char *key, int key_len, int hash_type, const unsigned char *data, unsigned int data_len, unsigned char *md, unsigned int *md_len);

private:
    HMAC_CTX ctx_;
};

#else

class SL_Crypto_HMAC
{
public:
    SL_Crypto_HMAC();
    virtual ~SL_Crypto_HMAC();

    int init(const unsigned char *key, int len, int hash_type);
    int update(const unsigned char *data, unsigned int len);
    int final(unsigned char *md, unsigned int *md_len);
    int final(const unsigned char *data, unsigned int len, unsigned char *md, unsigned int *md_len);

    static unsigned char* final(const unsigned char *key, int key_len, int hash_type, const unsigned char *data, unsigned int data_len, unsigned char *md, unsigned int *md_len);
};

#endif

#endif

