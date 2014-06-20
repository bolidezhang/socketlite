#ifndef SOCKETLITE_CRYPTO_SHA512_H
#define SOCKETLITE_CRYPTO_SHA512_H
#include "SL_Crypto_Hash.h"

#ifdef SOCKETLITE_USE_OPENSSL
#include <openssl/sha.h>

class SL_Crypto_SHA512 : public SL_Crypto_Hash
{
public:
    SL_Crypto_SHA512();
    virtual ~SL_Crypto_SHA512();

    int get_digest_len();
    int init();
    int update(const unsigned char *in, unsigned int in_len);
    int final(unsigned char *digest);
    int final(const unsigned char *in, unsigned int in_len, unsigned char *digest);

private:
    SHA512_CTX ctx_;
};

#else

class SL_Crypto_SHA512 : public SL_Crypto_Hash
{
public:
    SL_Crypto_SHA512();
    virtual ~SL_Crypto_SHA512();

    int get_digest_len();
    int init();
    int update(const unsigned char *in, unsigned int in_len);
    int final(unsigned char *digest);
    int final(const unsigned char *in, unsigned int in_len, unsigned char *digest);

};

#endif

#endif

