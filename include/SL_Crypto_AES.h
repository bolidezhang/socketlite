#ifndef SOCKETLITE_CRYPTO_AES_H
#define SOCKETLITE_CRYPTO_AES_H
#include "SL_Crypto_Cipher.h"

#ifdef SOCKETLITE_USE_OPENSSL
#include <openssl/aes.h>

class SL_Crypto_AES : public SL_Crypto_SymmetricCipher
{
public:
    SL_Crypto_AES();
    virtual ~SL_Crypto_AES();

    int get_need_len(unsigned int len, SL_Crypto_SymmetricCipher::TYPE type = SL_Crypto_SymmetricCipher::ENCRYPT);
    int init(const unsigned char *key, const unsigned char *ivec, int mode = SL_CRYPTO_MODE_ECB, int key_len = 16, int block_size = 16);
    int encrypt_block(const unsigned char *in, unsigned char *out);
    int decrypt_block(const unsigned char *in, unsigned char *out);
    int encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len);
    int decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len);

    //带模式加密解
    int encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len, unsigned char *ivec, int *num = NULL, unsigned char *ecount_buf = NULL);
    int decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len, unsigned char *ivec, int *num = NULL, unsigned char *ecount_buf = NULL);

private:
    AES_KEY encrypt_key_;
    AES_KEY decrypt_key_;
    int mode_;
};

#else

class SL_Crypto_AES : public SL_Crypto_SymmetricCipher
{
public:
    SL_Crypto_AES();
    virtual ~SL_Crypto_AES();

    int get_need_len(unsigned int len, SL_Crypto_SymmetricCipher::TYPE type = SL_Crypto_SymmetricCipher::ENCRYPT);
    int init(const unsigned char *key, const unsigned char *ivec, int mode = SL_CRYPTO_MODE_ECB, int key_len = 16, int block_size = 16);
    int encrypt_block(const unsigned char *in, unsigned char *out);
    int decrypt_block(const unsigned char *in, unsigned char *out);
    int encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len);
    int decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len);

    //带模式加密解
    int encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len, unsigned char *ivec, int *num = NULL, unsigned char *ecount_buf = NULL);
    int decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len, unsigned char *ivec, int *num = NULL, unsigned char *ecount_buf = NULL);

private:
    int mode_;
};
#endif

#endif

