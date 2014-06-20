#ifndef SOCKETLITE_CRYPTO_CRC16_H
#define SOCKETLITE_CRYPTO_CRC16_H
#include "SL_Crypto_Checksum.h"

class SL_Crypto_CRC16 : public SL_Crypto_Checksum
{
public:
    SL_Crypto_CRC16();
    ~SL_Crypto_CRC16();

    int  checksum_len();
    int  init();
    int  update(const unsigned char *data, uint len);
    int  final(unsigned char *checksum, uint len);
    int  final(const unsigned char *in, uint in_len, unsigned char *out, uint out_len);

    static uint16 checksum(const unsigned char *data, uint len, uint16 sum = 0);

private:
    uint16 checksum_;
};

#endif

