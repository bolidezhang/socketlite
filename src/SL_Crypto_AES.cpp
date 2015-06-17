#include <assert.h>
#include "SL_Crypto_AES.h"
#include "SL_Utility_Memory.h"

#ifdef SOCKETLITE_USE_OPENSSL
SL_Crypto_AES::SL_Crypto_AES()
    : mode_(SL_CRYPTO_MODE_ECB)
{
}

SL_Crypto_AES::~SL_Crypto_AES()
{
}

int SL_Crypto_AES::get_need_len(unsigned int len, SL_Crypto_SymmetricCipher::TYPE type)
{
    if (SL_Crypto_SymmetricCipher::ENCRYPT == type)
    {
        unsigned int quotient       = len >> 4;
        unsigned int end_block_len  = len - (quotient << 4);
        return (end_block_len ? (len - end_block_len + AES_BLOCK_SIZE + 1) : (len + 1));
    }
    else
    {
        return (len - 1);
    }
}

int SL_Crypto_AES::init(const unsigned char *key, const unsigned char *ivec, int mode, int key_len, int block_size)
{
    AES_set_encrypt_key(key, 8 * key_len, &encrypt_key_);
    AES_set_decrypt_key(key, 8 * key_len, &decrypt_key_);

    if ( (mode < SL_CRYPTO_MODE_ECB) || (mode > SL_CRYPTO_MODE_IGE) )
    {
        mode_ = SL_CRYPTO_MODE_ECB;
    }
    else
    {
        mode_ = mode;
    }
    return 0;
}

int SL_Crypto_AES::encrypt_block(const unsigned char *in, unsigned char *out)
{
    AES_encrypt(in, out, &encrypt_key_);
    return 0;
}

int SL_Crypto_AES::decrypt_block(const unsigned char *in, unsigned char *out)
{
    AES_decrypt(in, out, &decrypt_key_);
    return 0;
}

int SL_Crypto_AES::encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len)
{
    assert(in && out && (in_len > 0));

    unsigned int block_count    = in_len >> 4;
    unsigned int end_block_len  = in_len - (block_count << 4);
    unsigned int need_len       = end_block_len ? (in_len - end_block_len + AES_BLOCK_SIZE + 1) : (in_len + 1);
    if (out_len < need_len)
    {
        return -1;
    }

    unsigned char *in_pos    = (unsigned char *)in;
    unsigned char *out_pos   = (out + 1);
    for (unsigned int i = 0; i < block_count; ++i)
    {
        AES_encrypt(in_pos, out_pos, &encrypt_key_);
        in_pos  += AES_BLOCK_SIZE;
        out_pos	+= AES_BLOCK_SIZE;
    }
    if (end_block_len > 0)
    {            
        out[0] = AES_BLOCK_SIZE - end_block_len;
        unsigned char end_block[AES_BLOCK_SIZE] = {0};
        sl_memcpy(end_block, in_pos, end_block_len);
        AES_encrypt(end_block, out_pos, &encrypt_key_);
    }
    else
    {
        out[0] = 0;
    }

    return need_len;
}

int SL_Crypto_AES::decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len)
{
    assert(in && out && (in_len > AES_BLOCK_SIZE));

    unsigned int block_count = (in_len - 1) >> 4;
    unsigned int data_len    = block_count << 4;
    if ( (out_len < data_len) || (data_len + 1 != in_len) )
    {
        return -1;
    }

    unsigned char *in_pos    = (unsigned char *)(in + 1);
    unsigned char *out_pos   = out;
    for (unsigned int i=0; i<block_count; ++i)
    {
        AES_decrypt(in_pos, out_pos, &decrypt_key_);
        in_pos  += AES_BLOCK_SIZE;
        out_pos += AES_BLOCK_SIZE;
    }
    return (in_len - in[0] - 1);
}

int SL_Crypto_AES::encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len, unsigned char *ivec, int *num, unsigned char *ecount_buf)
{
    assert(in && out && (in_len > 0));

    unsigned int block_count    = in_len >> 4;
    unsigned int data_len       = block_count << 4;
    unsigned int end_block_len  = in_len - data_len;
    unsigned int need_len       = end_block_len ? (in_len - end_block_len + AES_BLOCK_SIZE + 1) : (in_len + 1);
    if (out_len < need_len)
    {
        return -1;
    }

    unsigned char *in_pos    = (unsigned char *)in;
    unsigned char *out_pos   = (out + 1);
    switch (mode_)
    {
        case SL_CRYPTO_MODE_ECB:
            {
                for (unsigned int i=0; i<block_count; ++i)
                {
                    AES_encrypt(in_pos, out_pos, &encrypt_key_);
                    in_pos += AES_BLOCK_SIZE;
                    out_pos += AES_BLOCK_SIZE;
                }
                if (end_block_len > 0)
                {            
                    out[0] = AES_BLOCK_SIZE - end_block_len;
                    unsigned char end_block[AES_BLOCK_SIZE] = {0};
                    sl_memcpy(end_block, in_pos, end_block_len);
                    AES_encrypt(end_block, out_pos, &encrypt_key_);
                }
                else
                {
                    out[0] = 0;
                }
            }
            break;
        case SL_CRYPTO_MODE_CBC:
            {
                if (end_block_len > 0)
                {
                    out[0] = AES_BLOCK_SIZE - end_block_len;
                    if (block_count > 0)
                    {
                        AES_cbc_encrypt(in_pos, out_pos, data_len, &encrypt_key_, ivec, 1);
                        in_pos  += data_len;
                        out_pos += data_len;
                    }
                    unsigned char end_block[AES_BLOCK_SIZE] = {0};
                    sl_memcpy(end_block, in_pos, end_block_len);
                    AES_cbc_encrypt(end_block, out_pos, AES_BLOCK_SIZE, &encrypt_key_, ivec, 1);
                }
                else
                {
                    out[0] = 0;
                    AES_cbc_encrypt(in_pos, out_pos, data_len, &encrypt_key_, ivec, 1);
                }
            }
            break;
        case SL_CRYPTO_MODE_CFB:
            {
                if (end_block_len > 0)
                {
                    out[0] = AES_BLOCK_SIZE - end_block_len;
                    if (block_count > 0)
                    {
                        AES_cfb128_encrypt(in_pos, out_pos, data_len, &encrypt_key_, ivec, num, 1);
                        in_pos  += data_len;
                        out_pos += data_len;
                    }
                    unsigned char end_block[AES_BLOCK_SIZE] = {0};
                    sl_memcpy(end_block, in_pos, end_block_len);
                    AES_cfb128_encrypt(end_block, out_pos, AES_BLOCK_SIZE, &encrypt_key_, ivec, num, 1);
                }
                else
                {
                    out[0] = 0;
                    AES_cfb128_encrypt(in_pos, out_pos, data_len, &encrypt_key_, ivec, (int *)num, 1);
                }
            }
            break;
        case SL_CRYPTO_MODE_OFB:
            {
                if (end_block_len > 0)
                {
                    out[0] = AES_BLOCK_SIZE - end_block_len;
                    if (block_count > 0)
                    {
                        AES_ofb128_encrypt(in_pos, out_pos, data_len, &encrypt_key_, ivec, num);
                        in_pos  += data_len;
                        out_pos += data_len;
                    }
                    unsigned char end_block[AES_BLOCK_SIZE] = {0};
                    sl_memcpy(end_block, in_pos, end_block_len);
                    AES_ofb128_encrypt(end_block, out_pos, AES_BLOCK_SIZE, &encrypt_key_, ivec, num);
                }
                else
                {
                    out[0] = 0;
                    AES_ofb128_encrypt(in_pos, out_pos, data_len, &encrypt_key_, ivec, num);
                }
            }
            break;
        case SL_CRYPTO_MODE_CTR:
            {
                if (end_block_len > 0)
                {
                    out[0] = AES_BLOCK_SIZE - end_block_len;
                    if (block_count > 0)
                    {
                        AES_ctr128_encrypt(in_pos, out_pos, data_len, &encrypt_key_, ivec, ecount_buf, (unsigned int *)num);
                        in_pos  += data_len;
                        out_pos += data_len;
                    }
                    unsigned char end_block[AES_BLOCK_SIZE] = {0};
                    sl_memcpy(end_block, in_pos, end_block_len);
                    AES_ctr128_encrypt(end_block, out_pos, AES_BLOCK_SIZE, &encrypt_key_, ivec, ecount_buf, (unsigned int *)num);
                }
                else
                {
                    out[0] = 0;
                    AES_ctr128_encrypt(in_pos, out_pos, data_len, &encrypt_key_, ivec, ecount_buf, (unsigned int *)num);
                }
            }
            break;
        case SL_CRYPTO_MODE_IGE:
            {
                if (end_block_len > 0)
                {
                    out[0] = AES_BLOCK_SIZE - end_block_len;
                    if (block_count > 0)
                    {
                        AES_ige_encrypt(in_pos, out_pos, data_len, &encrypt_key_, ivec, 1);
                        in_pos  += data_len;
                        out_pos += data_len;
                    }
                    unsigned char end_block[AES_BLOCK_SIZE] = {0};
                    sl_memcpy(end_block, in_pos, end_block_len);
                    AES_ige_encrypt(end_block, out_pos, AES_BLOCK_SIZE, &encrypt_key_, ivec, 1);
                }
                else
                {
                    out[0] = 0;
                    AES_ige_encrypt(in_pos, out_pos, data_len, &encrypt_key_, ivec, 1);
                }
            }
            break;
        default:
            return -1;
    }

    return need_len;
}

int SL_Crypto_AES::decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len, unsigned char *ivec, int *num, unsigned char *ecount_buf)
{
    assert(in && out && (in_len > AES_BLOCK_SIZE));

    unsigned int block_count = (in_len - 1) >> 4;
    unsigned int data_len    = block_count << 4;
    if ( (out_len < data_len) || (data_len + 1 != in_len) )
    {
        return -1;
    }

    unsigned char *in_pos    = (unsigned char *)(in + 1);
    unsigned char *out_pos   = out;
    switch (mode_)
    {
        case SL_CRYPTO_MODE_ECB:
            {
                for (unsigned int i=0; i<block_count; ++i)
                {
                    AES_decrypt(in_pos, out_pos, &decrypt_key_);
                    in_pos  += AES_BLOCK_SIZE;
                    out_pos += AES_BLOCK_SIZE;
                }
            }
            break;
        case SL_CRYPTO_MODE_CBC:
            AES_cbc_encrypt(in_pos, out_pos, data_len, &decrypt_key_, ivec, 0);
            break;
        case SL_CRYPTO_MODE_CFB:
            AES_cfb128_encrypt(in_pos, out_pos, data_len, &decrypt_key_, ivec, num, 0);
            break;
        case SL_CRYPTO_MODE_OFB:
            AES_ofb128_encrypt(in_pos, out_pos, data_len, &decrypt_key_, ivec, num);
            break;
        case SL_CRYPTO_MODE_CTR:
            AES_ctr128_encrypt(in_pos, out_pos, data_len, &decrypt_key_, ivec, ecount_buf, (unsigned int *)num);
            break;
        case SL_CRYPTO_MODE_IGE:
            AES_ige_encrypt(in_pos, out_pos, data_len, &decrypt_key_, ivec, 0);
            break;
        default:
            return -1;
    }

    return (in_len - in[0] - 1);
}

#else

SL_Crypto_AES::SL_Crypto_AES()
{
}

SL_Crypto_AES::~SL_Crypto_AES()
{
}

int SL_Crypto_AES::get_need_len(unsigned int len, SL_Crypto_SymmetricCipher::TYPE type)
{
    if (SL_Crypto_SymmetricCipher::ENCRYPT == type)
    {
        unsigned int quotient       = len >> 4;
        unsigned int end_block_len  = len - (quotient << 4);
        return (end_block_len ? (len - end_block_len + AES_BLOCK_SIZE + 1) : (len + 1));
    }
    else
    {
        return (len - 1);
    }
}

int SL_Crypto_AES::init(const unsigned char *key, const unsigned char *ivec, int mode, int key_len, int block_size)
{
    return 0;
}

int SL_Crypto_AES::encrypt_block(const unsigned char *in, unsigned char *out)
{
    return 0;
}

int SL_Crypto_AES::decrypt_block(const unsigned char *in, unsigned char *out)
{
    return 0;
}


int SL_Crypto_AES::encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len)
{
    return 0;
}

int SL_Crypto_AES::decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len)
{
    return 0;
}

int SL_Crypto_AES::encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len, unsigned char *ivec, int *num, unsigned char *ecount_buf)
{
    return 0;
}

int SL_Crypto_AES::decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len, unsigned char *ivec, int *num, unsigned char *ecount_buf)
{
    return 0;
}

#endif

