#ifndef SOCKETLITE_CRYPTO_OS_RAND_H
#define SOCKETLITE_CRYPTO_OS_RAND_H
#include "SL_Crypto_Rand.h"

#ifdef SOCKETLITE_OS_WINDOWS
    #include <Wincrypt.h>
    #pragma comment(lib, "Advapi32.lib")
#else
    #include <stdio.h>
    #include <fcntl.h>
#endif

class SL_Crypto_OSRand : public SL_Crypto_Rand
{
public:
    enum GENERATE_MODE
    {
        GENERATE_MODE_BLOCKING = 0,         //阻塞式产生随机数
        GENERATE_MODE_NON_BLOCKING = 1,     //非阻塞式产生随机数
    };

    SL_Crypto_OSRand();
    virtual ~SL_Crypto_OSRand();

    int     init(GENERATE_MODE mode = GENERATE_MODE_NON_BLOCKING);

    void    seed(int nseed = 0);
    int     random_int();
    int     random_int(int low, int high);
    double  random_double();
    double  random_double(double low, double high);
    int     random_byte(char *out, unsigned int out_len, SL_Crypto_Rand::TYPE type = SL_Crypto_Rand::RANDOM_CHAR, const char *special_char = NULL);
    int     random_byte(char *out, unsigned int out_len, int low, int high);

protected:
    // Randomizes a buffer
    void    fill_buffer_mt(char *buffer, unsigned int bytes);

#ifdef SOCKETLITE_OS_WINDOWS
    HCRYPTPROV crypt_prov_;
#else
    int        fd_;
#endif
};

#endif

