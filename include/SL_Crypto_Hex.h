#ifndef SOCKETLITE_CRYPTO_HEX_H
#define SOCKETLITE_CRYPTO_HEX_H
#include "SL_Crypto_BaseN.h"

//alias base16
class SL_Crypto_Hex : public SL_Crypto_BaseN
{
public:
    SL_Crypto_Hex();
    virtual ~SL_Crypto_Hex();

    void set_upper_case(bool upper = true);
    int  get_need_len(unsigned int len, SL_Crypto_BaseN::TYPE type = SL_Crypto_BaseN::ENCODE);
    int  encode(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int *out_len);
    int  encode(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len);
    int  decode(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int *out_len, bool checked_data = false);
    int  decode(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len,  bool checked_data = false);

private:
	unsigned char *encode_table_;
};

#endif

