#ifndef SOCKETLITE_CRYPTO_SHA256_H
#define SOCKETLITE_CRYPTO_SHA256_H
#include "SL_Crypto_Hash.h"

#ifdef SOCKETLITE_USE_OPENSSL
#include <openssl/sha.h>

class SL_Crypto_SHA256 : public SL_Crypto_Hash
{
public:
    SL_Crypto_SHA256();
    virtual ~SL_Crypto_SHA256();

    int get_digest_len();
    int init();
    int update(const unsigned char *in, unsigned int in_len);
    int final(unsigned char *digest);
    int final(const unsigned char *in, unsigned int in_len, unsigned char *digest);

private:
    SHA256_CTX ctx_;
};

#else
#include "sha256.h"
class SL_Crypto_SHA256 : public SL_Crypto_Hash
{
public:
    SL_Crypto_SHA256();
    virtual ~SL_Crypto_SHA256();

    int get_digest_len();
    int init();
    int update(const unsigned char *in, unsigned int in_len);
    int final(unsigned char *digest);
    int final(const unsigned char *in, unsigned int in_len, unsigned char *digest);

private:
    sha256_ctx ctx_;
};

#endif

#endif

