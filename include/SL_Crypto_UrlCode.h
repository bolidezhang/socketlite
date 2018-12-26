#ifndef SOCKETLITE_CRYTO_BASE64_H
#define SOCKETLITE_CRYTO_BASE64_H
#include "SL_Crypto_BaseN.h"

class SL_Crypto_UrlCode
{
public:
    enum ESCAPE
    {
        ESCAPE_URI = 0,
        ESCAPE_ARGS = 1,
        ESCAPE_URI_COMPONENT = 2,
        ESCAPE_HTML = 3,
        ESCAPE_REFRESH = 4,
        ESCAPE_MEMCACHED = 5,
        ESCAPE_MAIL_AUTH = 6,
    };

    enum UNESCAPE
    {
        UNESCAPE_NONE = 0,
        UNESCAPE_URI  = 1,
        UNESCAPE_REDIRECT = 2,
    };

    SL_Crypto_UrlCode();
    virtual ~SL_Crypto_UrlCode();

    static int get_need_len(unsigned int len, SL_Crypto_BaseN::TYPE type = SL_Crypto_BaseN::ENCODE);
    static int encode(const uchar *in, uint in_len, uchar *out, uint *out_len, ESCAPE type = ESCAPE_URI_COMPONENT);
    static int decode(const uchar *in, uint in_len, uchar *out, uint *out_len);
};

#endif

