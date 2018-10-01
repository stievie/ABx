#pragma once

/* Public domain */

#ifndef __ABCRYPT_SAFEBFUNS_H__
#define __ABCRYPT_SAFEBFUNS_H__

#include <string.h>

void explicit_bzero( void * const buf, const size_t n );
int timingsafe_bcmp( const void * const b1, const void * const b2, const size_t n );
int timingsafe_memcmp( const void * const b1, const void * const b2, const size_t len );

#endif
