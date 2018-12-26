#ifndef _SHA256_H
#define _SHA256_H

#ifndef SHA_UINT8
#define SHA_UINT8  unsigned char
#endif

#ifndef SHA_UINT32
#define SHA_UINT32 unsigned int
#endif

typedef struct
{
    SHA_UINT32 total[2];
    SHA_UINT32 state[8];
    SHA_UINT8  buffer[64];
} sha256_ctx;

void sha256_init(sha256_ctx *ctx);
void sha256_update(sha256_ctx *ctx, SHA_UINT8 *input, SHA_UINT32 length);
void sha256_final(sha256_ctx *ctx, SHA_UINT8 digest[32]);

#endif /* sha256.h */
