#pragma once

#ifndef __ABCRYPT_SHA1_H__
#define __ABCRYPT_SHA1_H__

#include <stdint.h>

typedef struct
{
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} sha1_ctx;

void sha1_transform(uint32_t state[5], const unsigned char buffer[64]);
void sha1_init(sha1_ctx* ctx);
void sha1_update(sha1_ctx* ctx, const unsigned char* data, uint32_t len);
void sha1_final(sha1_ctx* ctx, unsigned char digest[20]);
void sha1(const char* str, int len, char* hash_out);

#endif
