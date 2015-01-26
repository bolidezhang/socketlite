/*
* File: random.h
* Version: 1.0
* Last modified on Fri Jul 22 16:44:36 1994 by eroberts
* -----------------------------------------------------
* This interface provides several functions for generating
* pseudo-random numbers.
*/

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "SL_Crypto_CRTRand.h"

SL_Crypto_CRTRand::SL_Crypto_CRTRand()
{
    seed();
}

SL_Crypto_CRTRand::~SL_Crypto_CRTRand()
{
}

void SL_Crypto_CRTRand::seed(int nseed)
{
    if (0 == nseed)
    {
        ::srand((unsigned int)::time(NULL));
    }
    else
    {
        ::srand(nseed);
    }
}

int SL_Crypto_CRTRand::random_int()
{
    return rand();

    //int ret = ::rand();
    //if (ret > RAND_MAX/2)
    //{
    //	ret = ret<<16 | ret;
    //}
    //return ret;
}

int SL_Crypto_CRTRand::random_int(int low, int high)
{
    return SL_Crypto_Rand::random_int(low, high);
}

double SL_Crypto_CRTRand::random_double()
{
    return SL_Crypto_Rand::random_double();
}

double SL_Crypto_CRTRand::random_double(double low, double high)
{
    return SL_Crypto_Rand::random_double(low, high);
}

int SL_Crypto_CRTRand::random_byte(char *out, unsigned int out_len, SL_Crypto_Rand::TYPE type, const char *special_char)
{
    return SL_Crypto_Rand::random_byte(out, out_len, type, special_char);
}

int SL_Crypto_CRTRand::random_byte(char *out, unsigned int out_len, int low, int high)
{
    return SL_Crypto_Rand::random_byte(out, out_len, low, high);
}

/*
* Function: random_chance
* ----------------------
* This function uses random_double to generate a number
* between 0 and 100, which it then compares to p.
*/
bool SL_Crypto_CRTRand::random_chance(double p)
{
    return (SL_Crypto_Rand::random_double(0, 1) < p);
}

