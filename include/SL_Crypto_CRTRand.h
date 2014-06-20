#ifndef SOCKETLIETE_CRYPTO_CRTRAND_H
#define SOCKETLIETE_CRYPTO_CRTRAND_H
#include "SL_Crypto_Rand.h"

class SL_Crypto_CRTRand : public SL_Crypto_Rand
{
public:
    SL_Crypto_CRTRand();
    virtual ~SL_Crypto_CRTRand();

    void    seed(int nseed = 0);
    int     random_int();
    int     random_int(int low, int high);
    double  random_double();
    double  random_double(double low, double high);
    int     random_byte(char *out, unsigned int out_len, SL_Crypto_Rand::TYPE type = SL_Crypto_Rand::RANDOM_CHAR, const char *special_char = NULL);
    int     random_byte(char *out, unsigned int out_len, int low, int high);

    /*
    * Function: random_chance
    * Usage: if (random_chance(p)) . . .
    * ---------------------------------
    * The random_chance function returns TRUE with the probability
    * indicated by p, which should be a floating-point number between
    * 0 (meaning never) and 1 (meaning always).  For example, calling
    * random_chance(.30) returns TRUE 30 percent of the time.
    */
    bool random_chance(double p);
};

#endif

