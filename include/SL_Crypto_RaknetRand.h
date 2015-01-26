#ifndef SOCKETLITE_CRYPTO_RAKNET_RAND_H
#define SOCKETLITE_CRYPTO_RAKNET_RAND_H
#include "SL_Crypto_Rand.h"

class SL_Crypto_RaknetRand : public SL_Crypto_Rand
{
    // Initialise seed for Random Generator
    // param[in] seed The seed value for the random number generator.
    // extern void RAK_DLL_EXPORT seed_mt( unsigned int seed );
public:
    SL_Crypto_RaknetRand();
    virtual ~SL_Crypto_RaknetRand();

    void    seed(int nseed = 0);
    int     random_int();
    int     random_int(int low, int high);
    double  random_double();
    double  random_double(double low, double high);
    int     random_byte(char *out, unsigned int out_len, SL_Crypto_Rand::TYPE type = SL_Crypto_Rand::RANDOM_CHAR, const char *special_char = NULL);
    int     random_byte(char *out, unsigned int out_len, int low, int high);

protected:
    void seed_mt(unsigned int seed);

    // internal
    // extern unsigned int RAK_DLL_EXPORT reload_mt( void );
    unsigned int reload_mt();

    // Gets a random float
    // return 0 to 1.0f, inclusive
    float frandom_mt();

    unsigned int    state_[625];        // state vector(624) + 1 extra to not violate ANSI C
    unsigned int    *next_;             // next random value is computed from here
    int             left_;              // can *next++ this many times before reloading
};

#endif

