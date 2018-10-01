#pragma once

#ifndef __ABCRYPT_BCRYPT_H__
#define __ABCRYPT_BCRYPT_H__

#include "blowfish.h"

int bcrypt_hashpass(const char *key, const char *salt, char *encrypted,
    u_int16_t encryptedlen);
int bcrypt_newhash(const char *pass, int log_rounds, char *hash, u_int16_t hashlen);
int bcrypt_checkpass(const char *pass, const char *goodhash);

#endif
