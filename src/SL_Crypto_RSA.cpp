#include <assert.h>
#include "SL_Crypto_RSA.h"

#ifdef SOCKETLITE_USE_OPENSSL
#include <openssl/bn.h>

SL_Crypto_RSA::SL_Crypto_RSA()
    : key_rsa_(NULL)
    , public_key_rsa_(NULL)
    , private_key_rsa_(NULL)
    , e_(RSA_F4)
    , bits_(2048)
    , rsa_size_(256)
    , padding_(RSA_PKCS1_OAEP_PADDING)
    , sign_type_(NID_sha256)
{
}

SL_Crypto_RSA::~SL_Crypto_RSA()
{
    fini();
}

int SL_Crypto_RSA::init(int bits, unsigned long e, int padding, int sign_type)
{
    bits_       = bits;
    rsa_size_   = bits / 8;
    e_          = e;
    padding_    = padding;
    sign_type_  = sign_type;
    return 0;
}

int SL_Crypto_RSA::fini()
{
    if (NULL != key_rsa_)
    {
        if ( (NULL != public_key_rsa_) && (key_rsa_ != public_key_rsa_) )
        {
            RSA_free(public_key_rsa_);
        }
        if ( (NULL != private_key_rsa_) && (key_rsa_ != private_key_rsa_) )
        {
            RSA_free(private_key_rsa_);
        }
        RSA_free(key_rsa_);
        key_rsa_ = NULL;
        public_key_rsa_ = NULL;
        private_key_rsa_ = NULL;
    }
    else
    {
        if (NULL != public_key_rsa_)
        {
            RSA_free(public_key_rsa_);
            public_key_rsa_ = NULL;
        }
        if (NULL != private_key_rsa_)
        {
            RSA_free(private_key_rsa_);
            private_key_rsa_ = NULL;
        }
    }
    return 0;
}

int SL_Crypto_RSA::generate_key()
{
    RSA *key_rsa = RSA_new();
    if (NULL != key_rsa)
    {
        BIGNUM *bn_e = BN_new();
        if (NULL != bn_e)
        {
            BN_set_word(bn_e, e_);
            int ret = RSA_generate_key_ex(key_rsa, bits_, bn_e, NULL);
            BN_free(bn_e);
            if (ret >= 0)
            {            
                fini();
                key_rsa_ = key_rsa;
                public_key_rsa_ = key_rsa;
                private_key_rsa_ = key_rsa;
                return 1;
            }
        }
        RSA_free(key_rsa);
    }
    return -1;
}

int SL_Crypto_RSA::export_public_key(char public_key[], int key_len)
{
    if (NULL != public_key_rsa_)
    {
        unsigned char *public_key_ptr = (unsigned char *)public_key;
        int public_key_len = i2d_RSAPublicKey(public_key_rsa_, &public_key_ptr);
        return public_key_len;
    }
    return -1;
}

int SL_Crypto_RSA::export_private_key(char private_key[], int key_len)
{
    if (NULL != private_key_rsa_)
    {
        unsigned char *private_key_ptr = (unsigned char *)private_key;
        int private_key_len = i2d_RSAPrivateKey(private_key_rsa_, &private_key_ptr);
        return private_key_len;
    }
    return -1;
}

int SL_Crypto_RSA::import_public_key(const char public_key[], int key_len)
{
    unsigned char *public_key_ptr = (unsigned char *)public_key;
    RSA *public_key_rsa = d2i_RSAPublicKey(NULL, (const unsigned char **)&public_key_ptr, key_len);
    if (NULL != public_key_rsa)
    {
        fini();
        public_key_rsa_ = public_key_rsa;
        rsa_size_ = RSA_size(public_key_rsa);
        bits_ = rsa_size_ * 8;
        return 0;
    }
    return -1;
}

int SL_Crypto_RSA::import_private_key(const char private_key[], int key_len)
{
    unsigned char *private_key_ptr = (unsigned char *)private_key;
    RSA *private_key_rsa = d2i_RSAPrivateKey(NULL, (const unsigned char **)&private_key_ptr, key_len);
    if (NULL != private_key_rsa)
    {
        fini();
        private_key_rsa_ = private_key_rsa;
        rsa_size_ = RSA_size(private_key_rsa);
        bits_ = rsa_size_ * 8;
        return 0;
    }
    return -1;
}

int SL_Crypto_RSA::public_encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len)
{
    assert(public_key_rsa_);
    return RSA_public_encrypt(in_len, in, out, public_key_rsa_, padding_);
}

int SL_Crypto_RSA::private_decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len)
{
    assert(private_key_rsa_);
    return RSA_private_decrypt(in_len, in, out, private_key_rsa_, padding_);
}

int SL_Crypto_RSA::private_encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len)
{
    assert(private_key_rsa_);
    return RSA_private_encrypt(in_len, in, out, private_key_rsa_, padding_);
}

int SL_Crypto_RSA::public_decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len)
{
    assert(public_key_rsa_);
    return RSA_public_decrypt(in_len, in, out, public_key_rsa_, padding_);
}

int SL_Crypto_RSA::sign(const unsigned char *md, unsigned int md_len, unsigned char *sig_buf, unsigned int *sig_len)
{
    assert(private_key_rsa_);
    return RSA_sign(sign_type_, md, md_len, sig_buf, sig_len, private_key_rsa_);
}

int SL_Crypto_RSA::verify(const unsigned char *md, unsigned int md_len, const unsigned char *sig_buf, unsigned int sig_len)
{
    assert(public_key_rsa_);
    return RSA_verify(sign_type_, md, md_len, sig_buf, sig_len, public_key_rsa_);
}

int SL_Crypto_RSA::sign_ASN1_OCTET(const unsigned char *md, unsigned int md_len, unsigned char *sig_buf, unsigned int *sig_len)
{
    assert(private_key_rsa_);
    return RSA_sign_ASN1_OCTET_STRING(sign_type_, md, md_len, sig_buf, sig_len, private_key_rsa_);
}

int SL_Crypto_RSA::verify_ASN1_OCTET(const unsigned char *md, unsigned int md_len, unsigned char *sig_buf, unsigned int sig_len)
{
    assert(public_key_rsa_);
    return RSA_verify_ASN1_OCTET_STRING(sign_type_, md, md_len, sig_buf, sig_len, public_key_rsa_);
}

#else

SL_Crypto_RSA::SL_Crypto_RSA()
{
}

SL_Crypto_RSA::~SL_Crypto_RSA()
{
}

int SL_Crypto_RSA::init(int bits, unsigned long e, int padding, int sign_type)
{
    return 0;
}

int SL_Crypto_RSA::fini()
{
    return 0;
}

int SL_Crypto_RSA::generate_key()
{
    return -1;
}

int SL_Crypto_RSA::export_public_key(char public_key[], int key_len)
{
    return -1;
}

int SL_Crypto_RSA::export_private_key(char private_key[], int key_len)
{
    return -1;
}

int SL_Crypto_RSA::import_public_key(const char public_key[], int key_len)
{
    return -1;
}

int SL_Crypto_RSA::import_private_key(const char private_key[], int key_len)
{
    return -1;
}

int SL_Crypto_RSA::public_encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len)
{
    return -1;
}

int SL_Crypto_RSA::public_decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len)
{
    return -1;
}

int SL_Crypto_RSA::private_encrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len)
{
    return -1;
}

int SL_Crypto_RSA::private_decrypt(const unsigned char *in, unsigned int in_len, unsigned char *out, unsigned int out_len)
{
    return -1;
}

int SL_Crypto_RSA::sign(const unsigned char *md, unsigned int md_len, unsigned char *sig_buf, unsigned int *sig_len)
{
    return -1;
}

int SL_Crypto_RSA::verify(const unsigned char *md, unsigned int md_len, const unsigned char *sig_buf, unsigned int sig_len)
{
    return -1;
}

int SL_Crypto_RSA::sign_ASN1_OCTET(const unsigned char *md, unsigned int md_len, unsigned char *sig_buf, unsigned int *sig_len)
{
    return -1;
}

int SL_Crypto_RSA::verify_ASN1_OCTET(const unsigned char *md, unsigned int md_len, unsigned char *sig_buf, unsigned int sig_len)
{
    return -1;
}

#endif

