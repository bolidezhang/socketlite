#ifndef SOCKETLITE_CRYPTO_CRC64_H
#define SOCKETLITE_CRYPTO_CRC64_H
#include "SL_Crypto_Checksum.h"

class SL_Crypto_CRC64 : public SL_Crypto_Checksum
{
public:
    SL_Crypto_CRC64();
    ~SL_Crypto_CRC64();

    int  checksum_len();
    int  init();
    int  update(const unsigned char *data, uint len);
    int  final(unsigned char *checksum, uint len);
    int  final(const unsigned char *in, uint in_len, unsigned char *out, uint out_len);

    static uint64 checksum(const unsigned char *data, uint len, uint64 sum = 0);

private:
    uint64 checksum_;
};

#endif

