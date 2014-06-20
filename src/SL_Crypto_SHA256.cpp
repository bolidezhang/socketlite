#include "SL_Crypto_SHA256.h"

#ifdef SOCKETLITE_USE_OPENSSL
SL_Crypto_SHA256::SL_Crypto_SHA256()
{
    init();
}

SL_Crypto_SHA256::~SL_Crypto_SHA256()
{
}

int SL_Crypto_SHA256::get_digest_len()
{
    return SHA256_DIGEST_LENGTH;
}

int SL_Crypto_SHA256::init()
{
    return SHA256_Init(&ctx_);
}

int SL_Crypto_SHA256::update(const unsigned char *in, unsigned int in_len)
{
    return SHA256_Update(&ctx_, in, in_len);
}

int SL_Crypto_SHA256::final(unsigned char *digest)
{
    SHA256_Final(digest, &ctx_);

    //recover init status
    SHA256_Init(&ctx_);

    return 0;
}

int SL_Crypto_SHA256::final(const unsigned char *in, unsigned int in_len, unsigned char *digest)
{
    SHA256_Update(&ctx_, in, in_len);
    SHA256_Final(digest, &ctx_);

    //recover init status
    SHA256_Init(&ctx_);

    return 0;
}

#else

SL_Crypto_SHA256::SL_Crypto_SHA256()
{
}

SL_Crypto_SHA256::~SL_Crypto_SHA256()
{
}

int SL_Crypto_SHA256::get_digest_len()
{
    return 0;
}

int SL_Crypto_SHA256::init()
{
    return 0;
}

int SL_Crypto_SHA256::update(const unsigned char *in, unsigned int in_len)
{
	return 0;
}

int SL_Crypto_SHA256::final(unsigned char *digest)
{
	return 0;
}

int SL_Crypto_SHA256::final(const unsigned char *in, unsigned int in_len, unsigned char *digest)
{
    return 0;
}

#endif

