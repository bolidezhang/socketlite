#include "SL_Crypto_Rand.h"

#ifndef RAND_MAX
#define RAND_MAX ((int) ((unsigned) ~0 >> 1))
#endif

SL_Crypto_Rand::SL_Crypto_Rand()
{
}

SL_Crypto_Rand::~SL_Crypto_Rand()
{
}

int SL_Crypto_Rand::random_int(int low, int high)
{
    int distance = high - low;
    if (distance > 0)
    {
        return (low + (random_int() % (distance + 1)));
    }
    return low;
}

double SL_Crypto_Rand::random_double()
{
    return (double)(random_int() / ((double)RAND_MAX + 1));
}

double SL_Crypto_Rand::random_double(double low, double high)
{
    if (high < low)
    {
        double tmp = high;
        high = low;
        low  = tmp;
    };

    double d;
    d = (double)(random_int() / ((double)RAND_MAX + 1));
    return (low + d * (high - low));
}

int SL_Crypto_Rand::random_byte(char *out, unsigned int out_len, SL_Crypto_Rand::TYPE type, const char *special_char)
{
    if (type == SL_Crypto_Rand::RANDOM_CHAR)
    {
        fill_buffer_mt(out, out_len);
    }
    else if (type == SL_Crypto_Rand::VISIBLE_CHAR       ||  \
             type == SL_Crypto_Rand::STANDARD_CHAR      ||  \
             type == SL_Crypto_Rand::NUMBER_CHAR        ||  \
             type == SL_Crypto_Rand::UPPERCASE_CHAR     ||  \
             type == SL_Crypto_Rand::LOWERCASE_CHAR)
    {
        fill_buffer_mt(out, out_len, type);
    }
    else if ((type == SL_Crypto_Rand::ENLARGE_CHAR) && (special_char != NULL))
    {
        fill_buffer_mt(out, out_len, special_char);
    }
    else
    {
        fill_buffer_mt(out, out_len, SL_Crypto_Rand::STANDARD_CHAR);
    }

    return out_len;
}

int SL_Crypto_Rand::random_byte(char *out, unsigned int out_len, int low, int high)
{
    char n;
    for (unsigned int i=0; i<out_len; ++i)
    {
        n = random_int(low, high);
        *(out + i) = n;
    }
    return out_len;
}

char SL_Crypto_Rand::random_mt_char()
{
    char ret = random_int(48, 122);
    if (ret >=58 && ret <= 64)
    {
        ret = random_int(48, 57);
    }
    if (ret >= 91 && ret <= 96)
    {
        ret = random_int(97, 122);
    }
    return ret;
}

char SL_Crypto_Rand::random_mt_char(const char *special_char)
{
    char ret = random_int(48, 122);

    if (ret >=58 && ret <= 64)
    {
        int len = (int)strlen(special_char);
        int char_num = random_int(0, len-1);
        ret = special_char[char_num];
    }
    if (ret >= 91 && ret <= 96)
    {
        int len = (int)strlen(special_char);
        int char_num = random_int(0, len-1);
        ret = special_char[char_num];
    }

    return ret;
}

void SL_Crypto_Rand::fill_buffer_mt(char *buffer, unsigned int bytes)
{
    int n;
    for (unsigned int i=0; i<bytes; ++i)
    {
        n = random_int(0, 255);
        *(buffer + i) = n;
    }
}

void SL_Crypto_Rand::fill_buffer_mt(char *buffer, unsigned int bytes, int type)
{
    if ((type < SL_Crypto_Rand::RANDOM_CHAR) || (type >= SL_Crypto_Rand::TYPE_NUMS))
    {
        type = SL_Crypto_Rand::STANDARD_CHAR;
    }

    int n;
    switch (type)
    {
    case SL_Crypto_Rand::VISIBLE_CHAR:
        for (unsigned int i=0; i<bytes; ++i)
        {
            n = random_int(32, 126);
            *(buffer + i) = n;
        }
        break;
    case SL_Crypto_Rand::NUMBER_CHAR:
        for (unsigned int i=0; i<bytes; ++i)
        {
            n = random_int(48, 57);
            *(buffer + i) = n;
        }
        break;
    case SL_Crypto_Rand::UPPERCASE_CHAR:
        for (unsigned int i=0; i<bytes; ++i)
        {
            n = random_int(65, 90);
            *(buffer + i) = n;
        }
        break;
    case SL_Crypto_Rand::LOWERCASE_CHAR:
        for (unsigned int i=0; i<bytes; ++i)
        {
            n = random_int(97, 112);
            *(buffer + i) = n;
        }
        break;
    default:
        for (unsigned int i=0; i<bytes; ++i)
        {
            n = random_mt_char();
            *(buffer + i) = n;
        }
        break;
    }
}

void SL_Crypto_Rand::fill_buffer_mt(char *buffer, unsigned int bytes, const char *special_char)
{
    char n;
    for (unsigned int i=0; i<bytes; ++i)
    {
        n = random_mt_char(special_char);
        *(buffer + i) = n;
    }
}

