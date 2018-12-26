#include "SL_Crypto_SHA512.h"

#ifdef SOCKETLITE_USE_OPENSSL
SL_Crypto_SHA512::SL_Crypto_SHA512()
{
    init();
}

SL_Crypto_SHA512::~SL_Crypto_SHA512()
{
}

int SL_Crypto_SHA512::get_digest_len()
{
    return SHA512_DIGEST_LENGTH;
}

int SL_Crypto_SHA512::init()
{
    return SHA512_Init(&ctx_);
}

int SL_Crypto_SHA512::update(const unsigned char *in, unsigned int in_len)
{
    return SHA512_Update(&ctx_, in, in_len);
}

int SL_Crypto_SHA512::final(unsigned char *digest)
{
    SHA512_Final(digest, &ctx_);

    //recover init status
    //SHA512_Init(&ctx_);

    return 0;
}

int SL_Crypto_SHA512::final(const unsigned char *in, unsigned int in_len, unsigned char *digest)
{
    SHA512_Update(&ctx_, in, in_len);
    SHA512_Final(digest, &ctx_);

    //recover init status
    //SHA512_Init(&ctx_);

    return 0;
}

#else

SL_Crypto_SHA512::SL_Crypto_SHA512()
{
}

SL_Crypto_SHA512::~SL_Crypto_SHA512()
{
}

int SL_Crypto_SHA512::get_digest_len()
{
    return 0;
}

int SL_Crypto_SHA512::init()
{
    return 0;
}

int SL_Crypto_SHA512::update(const unsigned char *in, unsigned int in_len)
{
    return 0;
}

int SL_Crypto_SHA512::final(unsigned char *digest)
{
    return 0;
}

int SL_Crypto_SHA512::final(const unsigned char *in, unsigned int in_len, unsigned char *digest)
{
    return 0;
}

#endif

