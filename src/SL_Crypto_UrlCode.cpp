#include <assert.h>
#include "SL_Config.h"
#include "SL_Crypto_UrlCode.h"

SL_Crypto_UrlCode::SL_Crypto_UrlCode()
{
}

SL_Crypto_UrlCode::~SL_Crypto_UrlCode()
{
}

int SL_Crypto_UrlCode::get_need_len(unsigned int len, SL_Crypto_BaseN::TYPE type)
{
    return 0;
}

int SL_Crypto_UrlCode::encode(const uchar *in, uint in_len, uchar *out, uint *out_len, ESCAPE type)
{
    uint32       *escape = 0;
    static uchar hex[] = "0123456789ABCDEF";

    /* " ", "#", "%", "?", %00-%1F, %7F-%FF */
    static uint32   uri[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

        /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x80000029, /* 1000 0000 0000 0000  0000 0000 0010 1001 */

        /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

        /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x80000000, /* 1000 0000 0000 0000  0000 0000 0000 0000 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

    /* " ", "#", "%", "&", "+", "?", %00-%1F, %7F-%FF */
    static uint32   args[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

        /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x88000869, /* 1000 1000 0000 0000  0000 1000 0110 1001 */

        /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

        /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x80000000, /* 1000 0000 0000 0000  0000 0000 0000 0000 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

    /* not ALPHA, DIGIT, "-", ".", "_", "~" */
    static uint32   uri_component[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

        /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0xfc009fff, /* 1111 1100 0000 0000  1001 1111 1111 1111 */

        /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x78000001, /* 0111 1000 0000 0000  0000 0000 0000 0001 */

        /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0xb8000001, /* 1011 1000 0000 0000  0000 0000 0000 0001 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

    /* " ", "#", """, "%", "'", %00-%1F, %7F-%FF */
    static uint32   html[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

        /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x000000ad, /* 0000 0000 0000 0000  0000 0000 1010 1101 */

        /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

        /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x80000000, /* 1000 0000 0000 0000  0000 0000 0000 0000 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

    /* " ", """, "%", "'", %00-%1F, %7F-%FF */

    static uint32   refresh[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

        /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x00000085, /* 0000 0000 0000 0000  0000 0000 1000 0101 */

        /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

        /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x80000000, /* 1000 0000 0000 0000  0000 0000 0000 0000 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

    /* " ", "%", %00-%1F */

    static uint32   memcached[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

        /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x00000021, /* 0000 0000 0000 0000  0000 0000 0010 0001 */

        /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

        /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
    };

    /* mail_auth is the same as memcached */

    static uint32 *map[] = { uri, args, uri_component, html, refresh, memcached, memcached };
    escape = map[type];

    if (NULL == out)
    {
        /* find the number of the characters to be escaped */
        int n = 0;
        while (in_len--)
        {
            if (escape[*in >> 5] & (1 << (*in & 0x1f))) 
            {
                n++;
            }
            in++;
        }
        *out_len = n;
        return n;
    }

    unsigned char *pos = out;
    while (in_len--)
    {
        if (escape[*in >> 5] & (1 << (*in & 0x1f)))
        {
            *pos++ = '%';
            *pos++ = hex[*in >> 4];
            *pos++ = hex[*in & 0xf];
            in++;

        }
        else 
        {
            *pos++ = *in++;
        }
    }

    *out_len = (uint)(pos - out);
    return 1;
}

int SL_Crypto_UrlCode::decode(const uchar *in, uint in_len, uchar *out, uint *out_len)
{
    enum 
    {
        sw_usual = 0,
        sw_quoted,
        sw_quoted_second
    } state;

    state = sw_usual;
    unsigned char *out_temp = out;
    unsigned char *in_temp = (unsigned char *)in;
    int i = (int)in_len;
    uchar ch, c, decoded = 0;

    while (i--)
    {

        ch = *in_temp++;
        switch (state)
        {
        case sw_usual:
            if (ch == '%') 
            {
                state = sw_quoted;
                break;
            }

            *out_temp++ = ch;
            break;
        case sw_quoted:
            if (ch >= '0' && ch <= '9') 
            {
                decoded = (uchar)(ch - '0');
                state = sw_quoted_second;
                break;
            }

            c = (uchar)(ch | 0x20);
            if (c >= 'a' && c <= 'f')
            {
                decoded = (uchar)(c - 'a' + 10);
                state = sw_quoted_second;
                break;
            }

            /* the invalid quoted character */
            state = sw_usual;
            *out_temp++ = ch;
            break;
        case sw_quoted_second:
            state = sw_usual;
            if (ch >= '0' && ch <= '9') 
            {
                ch = (uchar)((decoded << 4) + ch - '0');
                *out_temp++ = ch;
                break;
            }

            c = (uchar)(ch | 0x20);
            if (c >= 'a' && c <= 'f')
            {
                ch = (uchar)((decoded << 4) + c - 'a' + 10);
                *out_temp++ = ch;
                break;
            }

            /* the invalid quoted character */
            break;
        }
    }

    *out_len = (uint)(out_temp - out);
    return (i < 0 && state == sw_usual);
}

