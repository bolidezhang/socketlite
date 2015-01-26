#include <assert.h>
#include "SL_Config.h"
#include "SL_Crypto_Base64.h"

static const unsigned char BASE64_ENCODE_TABLE[]    = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const unsigned char BASE64_DECODE_TABLE[256] = {
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x3e,0xff,0xff,0xff,0x3f,
    0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
    0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0xff,0xff,0xff,0xff,0xff,
    0xff,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
    0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
};

SL_Crypto_Base64::SL_Crypto_Base64()
{
}

SL_Crypto_Base64::~SL_Crypto_Base64()
{
}

int SL_Crypto_Base64::get_need_len(unsigned int len, SL_Crypto_BaseN::TYPE type)
{
    if (SL_Crypto_BaseN::ENCODE == type)
    {//encode
        unsigned int i = SL_MOD(len, 3);
        if (i)
        {
            return ((len + 3 - i) << 2) / 3;
        }
        return  (len << 2) / 3;
    }
    else
    {//decode
        return (len * 3) >> 2;
    }
}

int SL_Crypto_Base64::encode(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int *out_len)
{
    assert(in && out && out_len && (in_len > 0));

    unsigned int need_len;
    unsigned int remainder = SL_MOD(in_len, 3);
    switch (remainder)
    {
        case 1:
            {
                /* valid output size ? */
                need_len = ((in_len + 3 - remainder) << 2) / 3;
                if (*out_len < need_len)
                {
                    return -1;
                }

                unsigned int leven = in_len - remainder;
                for (unsigned int i = 0; i < leven; i += 3)
                {
                    *out++ = BASE64_ENCODE_TABLE[ (in[0] >> 2) & 0x3f ];
                    *out++ = BASE64_ENCODE_TABLE[((in[0] << 4) & 0x30) | ((in[1] >> 4) & 0x0f)];
                    *out++ = BASE64_ENCODE_TABLE[((in[1] << 2) & 0x3c) | ((in[2] >> 6) & 0x03)];
                    *out++ = BASE64_ENCODE_TABLE[  in[2] & 0x3f ];
                    in += 3;
                }

                *out++ = BASE64_ENCODE_TABLE[(in[0] >> 2) & 0x3f];
                *out++ = BASE64_ENCODE_TABLE[(in[0] << 4) & 0x30];
                *out++ = '=';
                *out++ = '=';
            }
            break;
        case 2:
            {
                /* valid output size ? */
                need_len = ((in_len + 3 - remainder) << 2) / 3;
                if (*out_len < need_len)
                {
                    return -1;
                }

                unsigned int leven = in_len - remainder;
                for (unsigned int i = 0; i < leven; i += 3)
                {
                    *out++ = BASE64_ENCODE_TABLE[ (in[0] >> 2) & 0x3f ];
                    *out++ = BASE64_ENCODE_TABLE[((in[0] << 4) & 0x30) | ((in[1] >> 4) & 0x0f)];
                    *out++ = BASE64_ENCODE_TABLE[((in[1] << 2) & 0x3c) | ((in[2] >> 6) & 0x03)];
                    *out++ = BASE64_ENCODE_TABLE[  in[2] & 0x3f ];
                    in += 3;
                }

                *out++ = BASE64_ENCODE_TABLE[ (in[0] >> 2) & 0x3f ];
                *out++ = BASE64_ENCODE_TABLE[((in[0] << 4) & 0x30) | ((in[1] >> 4) & 0x0f)];
                *out++ = BASE64_ENCODE_TABLE[ (in[1] << 2) & 0x3c ];
                *out++ = '=';
            }
            break;
        default:
            {//remainder == 0
                /* valid output size ? */
                need_len = (in_len << 2) / 3;
                if (*out_len < need_len)
                {
                    return -1;
                }

                for (unsigned int i = 0; i < in_len; i += 3)
                {
                    *out++ = BASE64_ENCODE_TABLE[ (in[0] >> 2) & 0x3f ];
                    *out++ = BASE64_ENCODE_TABLE[((in[0] << 4) & 0x30) | ((in[1] >> 4) & 0x0f)];
                    *out++ = BASE64_ENCODE_TABLE[((in[1] << 2) & 0x3c) | ((in[2] >> 6) & 0x03)];
                    *out++ = BASE64_ENCODE_TABLE[  in[2] & 0x3f ];
                    in += 3;
                }           
            }
        break;
    }

    /* return ok */
    *out_len = need_len;
    return need_len;
}

int SL_Crypto_Base64::encode(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len)
{
    return encode(in, in_len, out, (unsigned int *)&out_len);
}

int SL_Crypto_Base64::decode(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int *out_len, bool checked_data)
{
    assert(in && out && out_len && (in_len > 0));

    unsigned int need_len = (in_len * 3) >> 2;
    unsigned int mod_4 = in_len >> 2;
    mod_4 = in_len - (mod_4 << 2);
    if ( mod_4 || (*out_len < need_len) )
    {
        return -1;
    }

    //Calculate Padding number
    unsigned int  padding_number = 0;
    unsigned char *in1 = (unsigned char *)in + in_len - 1;
    while (*in1-- == '=')
    {
        ++padding_number;
    }

    switch (padding_number)
    {
        case 1:
            {
                need_len -= padding_number;
                in_len   -= 4;
                for (unsigned int i = 0; i < in_len; i += 4)
                {
                    *out++ = (BASE64_DECODE_TABLE[ in[0]] << 2)         | (BASE64_DECODE_TABLE[in[1]] >> 4);
                    *out++ = ((BASE64_DECODE_TABLE[in[1]] << 4) & 0xf0) | (BASE64_DECODE_TABLE[in[2]] >> 2);
                    *out++ = ((BASE64_DECODE_TABLE[in[2]] << 6) & 0xc0) | (BASE64_DECODE_TABLE[in[3]]);
                    in += 4;
                }
                *out++ = (BASE64_DECODE_TABLE[in[0]] << 2) | (BASE64_DECODE_TABLE[in[1]] >> 4);
                *out++ = ((BASE64_DECODE_TABLE[in[1]] << 4) & 0xf0) | (BASE64_DECODE_TABLE[in[2]] >> 2);
            }
            break;
        case 2:
            {
                need_len -= padding_number;
                in_len   -= 4;
                for (unsigned int i = 0; i < in_len; i += 4)
                {
                    *out++ = (BASE64_DECODE_TABLE[ in[0]] << 2)         | (BASE64_DECODE_TABLE[in[1]] >> 4);
                    *out++ = ((BASE64_DECODE_TABLE[in[1]] << 4) & 0xf0) | (BASE64_DECODE_TABLE[in[2]] >> 2);
                    *out++ = ((BASE64_DECODE_TABLE[in[2]] << 6) & 0xc0) | (BASE64_DECODE_TABLE[in[3]]);
                    in += 4;
                }
                *out++ = (BASE64_DECODE_TABLE[in[0]] << 2) | (BASE64_DECODE_TABLE[in[1]] >> 4);
            }
            break;
        default:
            {//padding_number == 0
                for (unsigned int i = 0; i < in_len; i += 4)
                {
                    *out++ = (BASE64_DECODE_TABLE[ in[0]] << 2)         | (BASE64_DECODE_TABLE[in[1]] >> 4);
                    *out++ = ((BASE64_DECODE_TABLE[in[1]] << 4) & 0xf0) | (BASE64_DECODE_TABLE[in[2]] >> 2);
                    *out++ = ((BASE64_DECODE_TABLE[in[2]] << 6) & 0xc0) | (BASE64_DECODE_TABLE[in[3]]);
                    in += 4;
                }
            }
            break;
    }

    *out_len = need_len;
    return need_len;
}

int SL_Crypto_Base64::decode(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len, bool checked_data)
{
    return decode(in, in_len, out, (unsigned int *)&out_len, checked_data);
}

