#ifndef _AES_H
#define _AES_H

#ifndef AES_UINT8
#define AES_UINT8  unsigned char
#endif

#ifndef AES_UINT32
#define AES_UINT32 unsigned int
#endif

#ifndef AES_BLOCK_SIZE
#define AES_BLOCK_SIZE 16
#endif

typedef struct
{
    AES_UINT32 erk[64];     /* encryption round keys */
    AES_UINT32 drk[64];     /* decryption round keys */
    int nr;                 /* number of rounds */
} aes_ctx;

int  aes_set_key(aes_ctx *ctx, AES_UINT8 *key, int nbits);
void aes_encrypt(aes_ctx *ctx, AES_UINT8 input[16], AES_UINT8 output[16]);
void aes_decrypt(aes_ctx *ctx, AES_UINT8 input[16], AES_UINT8 output[16]);

#endif /* aes.h */
