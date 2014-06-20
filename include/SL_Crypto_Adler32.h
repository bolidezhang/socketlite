#ifndef SOCKETLITE_CRYPTO_ADLER32_H
#define SOCKETLITE_CRYPTO_ADLER32_H
#include "SL_Crypto_Checksum.h"

class SL_Crypto_Adler32 : public SL_Crypto_Checksum
{
public:
    SL_Crypto_Adler32();
    ~SL_Crypto_Adler32();

    int  checksum_len();
    int  init();
    int  update(const unsigned char *data, uint len);
    int  final(unsigned char *checksum, uint len);
    int  final(const unsigned char *in, uint in_len, unsigned char *out, uint out_len);

    static uint32 checksum(const unsigned char *data, uint len, uint32 sum = 0);
private:
    uint32 checksum_;
};

#endif

