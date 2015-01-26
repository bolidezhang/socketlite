#include <assert.h>
#include "SL_Crypto_Hex.h"

static unsigned char HEX_ENCODE_TABLE_UPPER[]   = "0123456789ABCDEF";
static unsigned char HEX_ENCODE_TABLE_LOWER[]   = "0123456789abcdef";
static const char HEX_DECODE_TABLE[256]         = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,  
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,  
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,  
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
};

SL_Crypto_Hex::SL_Crypto_Hex()
    : encode_table_(HEX_ENCODE_TABLE_UPPER)
{
}

SL_Crypto_Hex::~SL_Crypto_Hex()
{
}

void SL_Crypto_Hex::set_upper_case(bool upper_case)
{
    if (upper_case)
    {//letter upper
        encode_table_   = HEX_ENCODE_TABLE_UPPER;
    }
    else
    {//letter lower
        encode_table_   = HEX_ENCODE_TABLE_LOWER;
    }
}

int SL_Crypto_Hex::get_need_len(unsigned int len, SL_Crypto_BaseN::TYPE type)
{
    if (SL_Crypto_BaseN::ENCODE == type)
    {//encode
        return (len << 1);
    }
    else
    {//decode
        return (len >> 1);
    }
}

int SL_Crypto_Hex::encode(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int *out_len)
{
    assert(in && out && out_len && (in_len > 0));

    unsigned int need_len = in_len << 1;
    if (*out_len < need_len)
    {
        return -1;
    }

    for (unsigned int i = 0; i < in_len; ++i)
    {
        *out++ = encode_table_[in[i] >> 4];
        *out++ = encode_table_[in[i] & 0x0f];
    }

    *out_len = need_len;
    return need_len;
}

int SL_Crypto_Hex::encode(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len)
{
    return encode(in, in_len, out, (unsigned int *)&out_len);
}

int SL_Crypto_Hex::decode(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int *out_len, bool checked_data)
{
    assert(in && out && out_len && (in_len > 0));

    unsigned int need_len = in_len >> 1;
    unsigned int mod_2 = in_len - (need_len << 1);
    if ( mod_2 || (need_len > *out_len) )
    {
        return -1;
    }

    if (!checked_data)
    {
        for (unsigned int i = 0; i < in_len; i += 2)
        {
            *out++ = (HEX_DECODE_TABLE[in[0]] << 4) + HEX_DECODE_TABLE[in[1]];
            in += 2;
        }
    }
    else
    {
        char b1; //解码后1byte的前4bit
        char b2; //解码后1byte的后4bit
        for (unsigned int i = 0; i < in_len; i += 2)
        {
            b1 = HEX_DECODE_TABLE[in[0]];
            b2 = HEX_DECODE_TABLE[in[1]];
            if ( (b1 < 0) || (b2 < 0) )
            {
                return -2;
            }

            *out++ = (b1 << 4) + b2;
            in += 2;
        }
    }

    *out_len = need_len;
    return need_len;
}

int SL_Crypto_Hex::decode(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len, bool checked_data)
{
    return decode(in, in_len, out, (unsigned int *)&out_len, checked_data);
}

