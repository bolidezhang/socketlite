#include <assert.h>
#include "SL_Crypto_OSRand.h"

SL_Crypto_OSRand::SL_Crypto_OSRand()
{    
#ifdef SOCKETLITE_OS_WINDOWS
    crypt_prov_ = NULL;
#else
    fd_ = -1;
#endif
    init();
}

SL_Crypto_OSRand::~SL_Crypto_OSRand()
{
#ifdef SOCKETLITE_OS_WINDOWS
    if (NULL != crypt_prov_)
    {
        CryptReleaseContext(crypt_prov_, 0);
        crypt_prov_ = NULL;
    }
#else
    if (fd_ > 0)
    {
        close(fd_);
        fd_ = -1;
    }
#endif
}

int SL_Crypto_OSRand::init(SL_Crypto_OSRand::GENERATE_MODE mode)
{    
#ifdef SOCKETLITE_OS_WINDOWS
    if (NULL == crypt_prov_)
    {
        if (CryptAcquireContext(&crypt_prov_, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
        {
            return 1;
        }
    }
#else
    if (fd_ > 0)
    {
        close(fd_);
    }
    if (GENERATE_MODE_BLOCKING == mode)
    {
        fd_ = open("/dev/random", O_RDONLY);
    }
    else
    {
        fd_ = open("/dev/urandom", O_RDONLY);
    }
    if (fd_ > 0)
    {
        return 1;
    }
#endif
    return 0;
}

void SL_Crypto_OSRand::seed(int nseed)
{
}

int SL_Crypto_OSRand::random_int()
{
    int n = 0;
    fill_buffer_mt((char *)&n, sizeof(int));
    return abs(n);
}

int SL_Crypto_OSRand::random_int(int low, int high)
{
    return SL_Crypto_Rand::random_int(low, high);
}

double SL_Crypto_OSRand::random_double()
{
    return SL_Crypto_Rand::random_double();
}

double SL_Crypto_OSRand::random_double(double low, double high)
{
    return SL_Crypto_Rand::random_double(low, high);
}

int SL_Crypto_OSRand::random_byte(char *out, unsigned int out_len, SL_Crypto_Rand::TYPE type, const char *special_char)
{
    assert(out && (out_len > 0));

    if (type == SL_Crypto_Rand::RANDOM_CHAR)
    {
        fill_buffer_mt(out, out_len);
    }
    else if (type == SL_Crypto_Rand::VISIBLE_CHAR   ||  \
             type == SL_Crypto_Rand::STANDARD_CHAR  ||  \
             type == SL_Crypto_Rand::NUMBER_CHAR    ||  \
             type == SL_Crypto_Rand::UPPERCASE_CHAR ||  \
             type == SL_Crypto_Rand::LOWERCASE_CHAR)
    {
        SL_Crypto_Rand::fill_buffer_mt(out, out_len, type);
    }
    else if ((type == SL_Crypto_Rand::ENLARGE_CHAR) && (special_char != NULL))
    {
        SL_Crypto_Rand::fill_buffer_mt(out, out_len, special_char);
    }
    else
    {
        SL_Crypto_Rand::fill_buffer_mt(out, out_len, SL_Crypto_Rand::STANDARD_CHAR);
    }
    return out_len;
}

int SL_Crypto_OSRand::random_byte(char *out, unsigned int out_len, int low, int high)
{
    return SL_Crypto_Rand::random_byte(out, out_len, low, high);
}

void SL_Crypto_OSRand::fill_buffer_mt(char *buffer, unsigned int bytes)
{
    assert(buffer && (bytes > 0));

#ifdef SOCKETLITE_OS_WINDOWS
    assert(NULL != crypt_prov_);
    CryptGenRandom(crypt_prov_, bytes, (BYTE *)buffer);
#else
    assert(fd_ > 0);

    int need_bytes  = bytes;
    int read_bytes  = 0;
    do 
    {
        read_bytes = read(fd_, buffer, need_bytes);
        if (read_bytes < 0)
        {
            return;
        }
        need_bytes -= read_bytes;
        buffer     += read_bytes;
    } while (need_bytes > 0);
#endif
}

