#pragma once

#ifndef __ABCRYPT_BUZ_H__
#define __ABCRYPT_BUZ_H__

#include <stddef.h>
#include <stdint.h>

uint32_t buzhash(const unsigned char *data, size_t len);
uint32_t buzhash_update(uint32_t sum, unsigned char remove, unsigned char add, size_t len);

#endif
