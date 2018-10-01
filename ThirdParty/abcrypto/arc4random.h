#pragma once

#ifndef __ABCRYPT_ARC4RANDOM_H__
#define __ABCRYPT_ARC4RANDOM_H__

#include <stdint.h>

/// Initialize
void arc4random_stir(void);

void arc4random_buf(void *_buf, size_t n);
unsigned int arc4random_uniform(unsigned int upper_bound);
unsigned int arc4random();

int earc4random();
int earc4random_m(int maxValue);
int earc4random_mm(int minValue, int maxValue);

#endif