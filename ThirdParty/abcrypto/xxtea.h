#pragma once

#ifndef __ABCRYPT_XXTEA_H__
#define __ABCRYPT_XXTEA_H__

#include <stdint.h>

void xxtea_enc(uint32_t* msg,
    const uint32_t n,
    const uint32_t *const key);
void xxtea_dec(uint32_t* msg,
    const uint32_t n,
    const uint32_t *const key);

#endif
