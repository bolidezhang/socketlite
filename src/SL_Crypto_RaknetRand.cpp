#include <stdlib.h>
#include <time.h>
#include <memory.h>
#include <math.h>
#include <string.h>
#include "SL_Crypto_RaknetRand.h"

#define RAKNET_RAND_N 				(624)     				// length of state vector
#define RAKNET_RAND_M               (397)                   // a period parameter
#define RAKNET_RAND_K               (0x9908B0DFU)           // a magic constant
#define RAKNET_RAND_hiBit(u)        ((u) & 0x80000000U)     // mask all but highest   bit of u
#define RAKNET_RAND_loBit(u)        ((u) & 0x00000001U)     // mask all but lowest    bit of u
#define RAKNET_RAND_loBits(u)       ((u) & 0x7FFFFFFFU)     // mask  the highest   bit of u
#define RAKNET_RAND_mixBits(u, v)   (RAKNET_RAND_hiBit(u)|RAKNET_RAND_loBits(v))    // move hi bit of u to hi bit of v

SL_Crypto_RaknetRand::SL_Crypto_RaknetRand()
{
	left_ = -1;
	seed();
}

SL_Crypto_RaknetRand::~SL_Crypto_RaknetRand()
{
}

void SL_Crypto_RaknetRand::seed(int nseed)
{
    if (0 == nseed)
    {
         ::srand((unsigned int)::time(NULL));
         nseed = rand();
    }
    seed_mt(nseed);
}

int SL_Crypto_RaknetRand::random_int()
{
    if (--left_ < 0)
    {
        return (reload_mt());
    }

    unsigned int y;
    y = *next_++;
    y ^= ( y >> 11 );
    y ^= ( y << 7 ) & 0x9D2C5680U;
    y ^= ( y << 15 ) & 0xEFC60000U;
    int ret = (y ^ ( y >> 18 ));
    return abs(ret);

    // This change made so the value returned is in the same range as what rand() returns
    // return(y ^ (y >> 18)) % 32767;
}

int SL_Crypto_RaknetRand::random_int(int low, int high)
{
    return SL_Crypto_Rand::random_int(low, high);
}

double SL_Crypto_RaknetRand::random_double()
{
    return frandom_mt();
}

double SL_Crypto_RaknetRand::random_double(double low, double high)
{
    if (low > high)
    {
        double temp;
        temp = low;
        low  = high;
        high = temp;
    }
    return low + frandom_mt() * (high - low);
}

int SL_Crypto_RaknetRand::random_byte(char *out, unsigned int out_len, SL_Crypto_Rand::TYPE type, const char *special_char)
{
    return SL_Crypto_Rand::random_byte(out, out_len, type, special_char);
}

int SL_Crypto_RaknetRand::random_byte(char *out, unsigned int out_len, int low, int high)
{
    return SL_Crypto_Rand::random_byte(out, out_len, low, high);
}

void SL_Crypto_RaknetRand::seed_mt(unsigned int seed)   // Defined in cokus_c.c
{
	//
	// We initialize state[0..(N-1)] via the generator
	//
	//  x_new = (69069 * x_old) mod 2^32
	//
	// from Line 15 of Table 1, p. 106, Sec. 3.3.4 of Knuth's
	// _The Art of Computer Programming_, Volume 2, 3rd ed.
	//
	// Notes (SJC): I do not know what the initial state requirements
	// of the Mersenne Twister are, but it seems this seeding generator
	// could be better.  It achieves the maximum period for its modulus
	// (2^30) iff x_initial is odd (p. 20-21, Sec. 3.2.1.2, Knuth); if
	// x_initial can be even, you have sequences like 0, 0, 0, ...;
	// 2^31, 2^31, 2^31, ...; 2^30, 2^30, 2^30, ...; 2^29, 2^29 + 2^31,
	// 2^29, 2^29 + 2^31, ..., etc. so I force seed to be odd below.
	//
	// Even if x_initial is odd, if x_initial is 1 mod 4 then
	//
	//  the   lowest bit of x is always 1,
	//  the  next-to-lowest bit of x is always 0,
	//  the 2nd-from-lowest bit of x alternates   ... 0 1 0 1 0 1 0 1 ... ,
	//  the 3rd-from-lowest bit of x 4-cycles   ... 0 1 1 0 0 1 1 0 ... ,
	//  the 4th-from-lowest bit of x has the 8-cycle ... 0 0 0 1 1 1 1 0 ... ,
	//   ...
	//
	// and if x_initial is 3 mod 4 then
	//
	//  the   lowest bit of x is always 1,
	//  the  next-to-lowest bit of x is always 1,
	//  the 2nd-from-lowest bit of x alternates   ... 0 1 0 1 0 1 0 1 ... ,
	//  the 3rd-from-lowest bit of x 4-cycles   ... 0 0 1 1 0 0 1 1 ... ,
	//  the 4th-from-lowest bit of x has the 8-cycle ... 0 0 1 1 1 1 0 0 ... ,
	//   ...
	//
	// The generator's potency (min. s>=0 with (69069-1)^s = 0 mod 2^32) is
	// 16, which seems to be alright by p. 25, Sec. 3.2.1.3 of Knuth.  It
	// also does well in the dimension 2..5 spectral tests, but it could be
	// better in dimension 6 (Line 15, Table 1, p. 106, Sec. 3.3.4, Knuth).
	//
	// Note that the random number user does not see the values generated
	// here directly since reloadMT() will always munge them first, so maybe
	// none of all of this matters.  In fact, the seed values made here could
	// even be extra-special desirable if the Mersenne Twister theory says
	// so-- that's why the only change I made is to restrict to odd seeds.
	//

	register unsigned int x = ( seed | 1U ) & 0xFFFFFFFFU, *s = state_;
	register int j;

	for (left_ = 0, *s++ = x, j = RAKNET_RAND_N; --j; *s++ = ( x *= 69069U ) & 0xFFFFFFFFU);
}

unsigned int SL_Crypto_RaknetRand::reload_mt()
{
	register unsigned int * p0 = state_, *p2 = state_ + 2, *pM = state_ + RAKNET_RAND_M, s0, s1;
	register int j;

	if (left_ < -1)
	{
		seed_mt(4357U);
	}

	left_ = RAKNET_RAND_N - 1, next_ = state_ + 1;

	for (s0 = state_[ 0 ], s1 = state_[ 1 ], j = RAKNET_RAND_N - RAKNET_RAND_M + 1; --j; s0 = s1, s1 = *p2++)
	{
		* p0++ = *pM++ ^ ( RAKNET_RAND_mixBits( s0, s1 ) >> 1 ) ^ ( RAKNET_RAND_loBit( s1 ) ? RAKNET_RAND_K : 0U );
	}

	for (pM = state_, j = RAKNET_RAND_M; --j; s0 = s1, s1 = *p2++)
	{
		* p0++ = *pM++ ^ ( RAKNET_RAND_mixBits( s0, s1 ) >> 1 ) ^ ( RAKNET_RAND_loBit( s1 ) ? RAKNET_RAND_K : 0U );
	}

	s1 = state_[ 0 ], *p0 = *pM ^ ( RAKNET_RAND_mixBits( s0, s1 ) >> 1 ) ^ ( RAKNET_RAND_loBit( s1 ) ? RAKNET_RAND_K : 0U );
	s1 ^= ( s1 >> 11 );
	s1 ^= ( s1 << 7 ) & 0x9D2C5680U;
	s1 ^= ( s1 << 15 ) & 0xEFC60000U;
	return ( s1 ^ ( s1 >> 18 ) );
}

float SL_Crypto_RaknetRand::frandom_mt()
{
	return (float) ((double) random_int() / 4294967296.0);
}

