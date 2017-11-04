#pragma once

#ifndef __ABCRYPT_MD5_H__
#define __ABCRYPT_MD5_H__

#include <stdint.h>

/*
Usage:
    unsigned char digest[16];
    md5_ctx ctx;
    md5_init(&ctx);
    md5_update(&ctx, string, strlen(string));
    md5_final(digest, &ctx);
*/

typedef unsigned int md5_u32;

typedef struct
{
    uint32_t lo, hi;
    uint32_t a, b, c, d;
    unsigned char buffer[64];
    uint32_t block[16];
} md5_ctx;

void md5_init(md5_ctx* ctx);
void md5_update(md5_ctx* ctx, const void* data, unsigned long size);
void md5_final(unsigned char* result, md5_ctx* ctx);
void md5(const char* buff, int len, unsigned char* hash);

#endif