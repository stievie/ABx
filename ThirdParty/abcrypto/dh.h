#pragma once

#ifndef __ABCRYPT_DH_H__
#define __ABCRYPT_DH_H__

#include <stddef.h>
#include <stdlib.h>

// 128 Bit key
#define DH_KEY_LENGTH	(16)

typedef unsigned char DH_KEY[DH_KEY_LENGTH];

void DH_generate_key_pair(DH_KEY public_key, DH_KEY private_key);
void DH_generate_key_secret(DH_KEY secret_key, const DH_KEY my_private, const DH_KEY another_public);
void DH_rand_buf(void* buf, size_t length);

#endif
