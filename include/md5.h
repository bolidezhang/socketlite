#ifndef _MD5_H
#define _MD5_H
#include <stddef.h>

#ifndef MD5_UINT8
#define MD5_UINT8  unsigned char
#endif

#ifndef MD5_UINT32
#define MD5_UINT32 unsigned int
#endif

#ifndef MD5_UINT64
#define MD5_UINT64 unsigned long long
#endif

typedef struct 
{
    MD5_UINT64  bytes;
    MD5_UINT32  a, b, c, d;
    MD5_UINT8   buffer[64];
} md5_ctx;


void md5_init(md5_ctx *ctx);
void md5_update(md5_ctx *ctx, const MD5_UINT8 *data, size_t size);
void md5_final(md5_ctx *ctx, MD5_UINT8 result[16]);

#endif /* _MD5_H */
