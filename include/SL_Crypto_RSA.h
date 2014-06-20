#ifndef SOCKETLITE_CRYPTO_RSA_H
#define SOCKETLITE_CRYPTO_RSA_H
#include "SL_Crypto_Cipher.h"

#ifdef SOCKETLITE_USE_OPENSSL
#include <openssl/obj_mac.h>
#include <openssl/rsa.h>

class SL_Crypto_RSA
{
public:
    SL_Crypto_RSA();
    virtual ~SL_Crypto_RSA();

    int init(int bits = 2048, unsigned long e = RSA_F4, int padding = RSA_PKCS1_PADDING, int sign_type = NID_sha256);
    int fini();

    int generate_key();
    int export_public_key(char public_key[], int key_len);
    int export_private_key(char private_key[], int key_len);
    int import_public_key(const char public_key[], int key_len);
    int import_private_key(const char private_key[], int key_len);

    //公钥加密/私钥解密
    int public_encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len);
    int private_decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len);

    //私钥加密/公钥解密
    int private_encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len);
    int public_decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len);

    //签名(私钥加密)/验证(公钥解密) X509_SIG ASN1
    int sign(const unsigned char *md, unsigned int md_len, unsigned char *sig_buf, unsigned int *sig_len);
    int verify(const unsigned char *md, unsigned int md_len, const unsigned char *sig_buf, unsigned int sig_len);

    //签名(私钥加密)/验证(公钥解密) ASN1_OCTET_STRING
    int sign_ASN1_OCTET(const unsigned char *md, unsigned int md_len, unsigned char *sig_buf, unsigned int *sig_len);
    int verify_ASN1_OCTET(const unsigned char *md, unsigned int md_len, unsigned char *sig_buf, unsigned int sig_len);

private:
    RSA *key_rsa_;
    RSA *public_key_rsa_;
    RSA *private_key_rsa_;

    unsigned long e_;
    int bits_;
    int rsa_size_;
    int padding_;
    int sign_type_;
};

#else

class SL_Crypto_RSA
{
public:
    SL_Crypto_RSA();
    virtual ~SL_Crypto_RSA();

    int init(int bits, unsigned long e, int padding);
    int fini();

    int generate_key();
    int export_public_key(char public_key[], int key_len);
    int export_private_key(char private_key[], int key_len);
    int import_public_key(const char public_key[], int key_len);
    int import_private_key(const char private_key[], int key_len);

    //公钥加密/私钥解密
    int public_encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len);
    int private_decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len);

    //私钥加密/公钥解密
    int private_encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len);
    int public_decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len);

    //签名(私钥加密)/验证(公钥解密) X509_SIG ASN1
    int sign(const unsigned char *md, unsigned int md_len, unsigned char *sig_buf, unsigned int *sig_len);
    int verify(const unsigned char *md, unsigned int md_len, const unsigned char *sig_buf, unsigned int sig_len);

    //签名(私钥加密)/验证(公钥解密) ASN1_OCTET_STRING
    int sign_ASN1_OCTET(const unsigned char *md, unsigned int md_len, unsigned char *sig_buf, unsigned int *sig_len);
    int verify_ASN1_OCTET(const unsigned char *md, unsigned int md_len, unsigned char *sig_buf, unsigned int sig_len);
};

#endif

#endif

