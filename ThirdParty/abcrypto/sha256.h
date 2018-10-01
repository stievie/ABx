#pragma once

#ifndef __ABCRYPT_SHA256_H__
#define __ABCRYPT_SHA256_H__

#include <stdint.h>

typedef struct
{
    uint32_t hash[8];
    unsigned char data[64];
    uint32_t total_len;
    uint32_t offset;
} sha256_ctx;

void sha256_init(sha256_ctx* context);
void sha256_update(sha256_ctx* context, const unsigned char *buf, unsigned len);
void sha256_final(sha256_ctx* context, unsigned char hash[32]);
void sha256(const char* str, int len, char* hash_out);

#endif