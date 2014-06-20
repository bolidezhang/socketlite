#ifndef SOCKETLITE_CRYPTO_CHECKSUM_H
#define SOCKETLITE_CRYPTO_CHECKSUM_H
#include "SL_Config.h"

class SL_Crypto_Checksum
{
public:
    virtual int  checksum_len() = 0;
    virtual int  init() = 0;
    virtual int  update(const unsigned char *data, uint len) = 0;
    virtual int  final(unsigned char *checksum, uint len) = 0;
    virtual int  final(const unsigned char *in, uint in_len, unsigned char *out, uint out_len) = 0;
};

#endif

