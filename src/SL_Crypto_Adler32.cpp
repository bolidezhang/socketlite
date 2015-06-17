#include <assert.h>
#include "SL_Crypto_Adler32.h"
#include "SL_Utility_Memory.h"

#define ADLER32_BASE 65521      /* largest prime smaller than 65536 */
#define ADLER32_NMAX 5552
/* ADLER32_NMAX is the largest n such that 255n(n+1)/2 + (n+1)(ADLER32_BASE-1) <= 2^32-1 */

#define DO1(buf,i)  {sum += (buf)[i]; sum2 += sum;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

/* use NO_DIVIDE if your processor does not do division in hardware --
   try it both ways to see which is faster */
#ifdef NO_DIVIDE
/* note that this assumes ADLER32_BASE is 65521, where 65536 % 65521 == 15
   (thank you to John Reiser for pointing this out) */
#  define CHOP(a) \
    do { \
        uint32 tmp = a >> 16; \
        a &= 0xffffUL; \
        a += (tmp << 4) - tmp; \
    } while (0)
#  define MOD28(a) \
    do { \
        CHOP(a); \
        if (a >= ADLER32_BASE) a -= ADLER32_BASE; \
    } while (0)
#  define MOD(a) \
    do { \
        CHOP(a); \
        MOD28(a); \
    } while (0)
#  define MOD63(a) \
    do { /* this assumes a is not negative */ \
        uint64 tmp = a >> 32; \
        a &= 0xffffffffL; \
        a += (tmp << 8) - (tmp << 5) + tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        if (a >= ADLER32_BASE) a -= ADLER32_BASE; \
    } while (0)
#else
#  define MOD(a) a %= ADLER32_BASE
#  define MOD28(a) a %= ADLER32_BASE
#  define MOD63(a) a %= ADLER32_BASE
#endif

/* ========================================================================= */
static uint32 adler32_combine_(uint32 adler1, uint32 adler2, uint64 len2)
{
    uint32 sum1;
    uint32 sum2;
    uint32 rem;

    /* for negative len, return invalid adler32 as a clue for debugging */
    if (len2 < 0)
    {
        return 0xffffffffUL;
    }

    /* the derivation of this formula is left as an exercise for the reader */
    MOD63(len2);                /* assumes len2 >= 0 */
    rem = (uint32)len2;
    sum1 = adler1 & 0xffff;
    sum2 = rem * sum1;
    MOD(sum2);
    sum1 += (adler2 & 0xffff) + ADLER32_BASE - 1;
    sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + ADLER32_BASE - rem;
    if (sum1 >= ADLER32_BASE) 
    {
        sum1 -= ADLER32_BASE;
    }
    if (sum1 >= ADLER32_BASE) 
    {
        sum1 -= ADLER32_BASE;
    }
    if (sum2 >= (ADLER32_BASE << 1)) 
    {
        sum2 -= (ADLER32_BASE << 1);
    }
    if (sum2 >= ADLER32_BASE) 
    {
        sum2 -= ADLER32_BASE;
    }
    return sum1 | (sum2 << 16);
}

/* ========================================================================= */
static uint32 adler32_combine(uint32 adler1, uint32 adler2, uint32 len2)
{
    return adler32_combine_(adler1, adler2, len2);
}

static uint32 adler32_combine64(uint32 adler1, uint32 adler2, uint64 len2)
{
    return adler32_combine_(adler1, adler2, len2);
}

SL_Crypto_Adler32::SL_Crypto_Adler32()
    : checksum_(0)
{
}

SL_Crypto_Adler32::~SL_Crypto_Adler32()
{
}

int SL_Crypto_Adler32::checksum_len()
{
    return sizeof(uint32);
}

int SL_Crypto_Adler32::init()
{
    checksum_ = 0;
    return 0;
}

int SL_Crypto_Adler32::update(const unsigned char *data, uint len)
{
    checksum_ = checksum(data, len, checksum_);
    return 0;
}

int SL_Crypto_Adler32::final(unsigned char *checksum, uint len)
{
    assert(checksum && (len >= sizeof(uint32)));

    sl_memcpy(checksum, (char *)&checksum_, sizeof(uint32));
    checksum_ = 0;
    return sizeof(uint32);
}

int SL_Crypto_Adler32::final(const unsigned char *in, uint in_len, unsigned char *out, uint out_len)
{
    assert(out && (out_len >= sizeof(uint32)));

    checksum_ = checksum(in, in_len, checksum_);
    sl_memcpy(out, (char *)&checksum_, sizeof(uint32));
    checksum_ = 0;
    return sizeof(uint32);
}

uint32 SL_Crypto_Adler32::checksum(const unsigned char *data, uint len, uint32 sum)
{
    uint32 sum2;
    uint32 n;

    /* split Adler-32 into component sums */
    sum2 = (sum >> 16) & 0xffff;
    sum &= 0xffff;

    /* in case user likes doing a byte at a time, keep it fast */
    if (1 == len) 
    {
        sum += data[0];
        if (sum >= ADLER32_BASE)
        {
            sum -= ADLER32_BASE;
        }
        sum2 += sum;
        if (sum2 >= ADLER32_BASE)
        {
            sum2 -= ADLER32_BASE;
        }
        return sum | (sum2 << 16);
    }

    /* initial Adler-32 value (deferred check for len == 1 speed) */
    if (NULL == data)
    {
        return 1L;
    }

    /* in case short lengths are provided, keep it somewhat fast */
    if (len < 16) 
    {
        while (len--) 
        {
            sum += *data++;
            sum2 += sum;
        }
        if (sum >= ADLER32_BASE)
        {
            sum -= ADLER32_BASE;
        }
        MOD28(sum2);            /* only added so many ADLER32_BASE's */
        return sum | (sum2 << 16);
    }

    /* do length ADLER32_NMAX blocks -- requires just one modulo operation */
    while (len >= ADLER32_NMAX) 
    {
        len -= ADLER32_NMAX;
        n = ADLER32_NMAX / 16;          /* ADLER32_NMAX is divisible by 16 */
        do 
        {
            DO16(data);          /* 16 sums unrolled */
            data += 16;
        } while (--n);
        MOD(sum);
        MOD(sum2);
    }

    /* do remaining bytes (less than ADLER32_NMAX, still just one modulo) */
    if (len) 
    {/* avoid modulos if none remaining */
        while (len >= 16) 
        {
            len -= 16;
            DO16(data);
            data += 16;
        }
        while (len--) 
        {
            sum += *data++;
            sum2 += sum;
        }
        MOD(sum);
        MOD(sum2);
    }

    /* return recombined sums */
    return sum | (sum2 << 16);
}

