#ifndef SOCKETLITE_CRYPTO_AES_H
#define SOCKETLITE_CRYPTO_AES_H
#include "SL_Crypto_Cipher.h"

#ifdef SOCKETLITE_USE_OPENSSL
#include <openssl/dh.h>
class SL_Crypto_DH
{
public:
    SL_Crypto_DH();
    virtual ~SL_Crypto_DH();
};

#else

class SL_Crypto_DH
{
public:
    SL_Crypto_DH();
    virtual ~SL_Crypto_DH();
};

#endif

#endif

