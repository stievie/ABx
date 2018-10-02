/*	$OpenBSD: arc4random.c,v 1.24 2013/06/11 16:59:50 deraadt Exp $	*/

/*
* Copyright (c) 1996, David Mazieres <dm@uun.org>
* Copyright (c) 2008, Damien Miller <djm@openbsd.org>
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/*
* Arc4 random number generator for OpenBSD.
*
* This code is derived from section 17.1 of Applied Cryptography,
* second edition, which describes a stream cipher allegedly
* compatible with RSA Labs "RC4" cipher (the actual description of
* which is a trade secret).  The same algorithm is used as a stream
* cipher called "arcfour" in Tatu Ylonen's ssh package.
*
* RC4 is a registered trademark of RSA Laboratories.
*/

#include "arc4random.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <Windows.h>
#endif
#if (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/time.h>
#endif

typedef uint8_t  u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;

typedef unsigned char u_char;

struct arc4_stream {
    u_int8_t i;
    u_int8_t j;
    u_int8_t s[256];
};


static int rs_initialized;
static struct arc4_stream rs;
static int arc4_count;

static inline u_int8_t arc4_getbyte(void);

static inline void arc4_init(void)
{
    int     n;

    for (n = 0; n < 256; n++)
        rs.s[n] = n;
    rs.i = 0;
    rs.j = 0;
}

static inline void arc4_addrandom(u_char *dat, int datlen)
{
    int     n;
    u_int8_t si;

    rs.i--;
    for (n = 0; n < 256; n++) {
        rs.i = (rs.i + 1);
        si = rs.s[rs.i];
        rs.j = (rs.j + si + dat[n % datlen]);
        rs.s[rs.i] = rs.s[rs.j];
        rs.s[rs.j] = si;
    }
    rs.j = rs.i;
}

static const char * urandom = "/dev/urandom";

static void fillRandom(u_char *rnd, size_t	len)
{
    for (int i = 0; i < (int)len; ++i)
        rnd[i] = i & 255;

    for (int k = 0; k < (int)len * 8; ++k)
    {
        int v1 = rand() % len;
        int v2 = rand() % len;
        u_char temp = rnd[v1];
        rnd[v1] = rnd[v2];
        rnd[v2] = temp;
    }
}

static void arc4_stir(void)
{
    int     i;
    u_char rnd[128];

    if (!rs_initialized) {
        arc4_init();
        rs_initialized = 1;
    }

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)

    HMODULE hLib = LoadLibraryA("ADVAPI32.DLL");
    if (hLib) {
        BOOLEAN(APIENTRY *pfn)(void*, ULONG) = (BOOLEAN(APIENTRY *)(void*, ULONG))GetProcAddress(hLib, "SystemFunction036");
        if (pfn) {
            if (pfn(rnd, sizeof(rnd))) {
                // use buff full of random goop
            }
        }
        FreeLibrary(hLib);
    }

#elif (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
    int n;
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != (-1))
    {

        /* Initialize the first element so it's hopefully not '0',
        * to help out the next loop. Tossing in some prime numbers
        * probably can't hurt. */
        rnd[0] = (tv.tv_sec % 10000) * 3 + tv.tv_usec * 7 + \
            (getpid() % 1000) * 13;

        for (n = 1; n < 127; n++)
        {

            /* Take advantage of the stack space. Only initialize
            * elements equal to '0'. This will make the rnd[]
            * array much less vulnerable to timing attacks. Here
            * we'll stir getpid() into the value of the previous
            * element. Approximately 1 in 128 elements will still
            * become '0'. */

            if (rnd[n] == 0)
            {
                rnd[n] = ((rnd[n - 1] + n) ^ \
                    ((getpid() % 1000) * 17));
            }
        }
    }
    else
    {
        /* gettimeofday() failed? Do the same thing as above, but only
        * with getpid(). */

        rnd[0] = (getpid() % 1000) * 19;
        for (n = 1; n < 127; n++)
        {
            if (rnd[n] == 0)
            {
                rnd[n] = ((rnd[n - 1] + n) ^ \
                    ((getpid() % 1000) * 23));
            }
        }
    }
#else

    size_t	len = sizeof(rnd);
    fillRandom(rnd, len);

#endif

    arc4_addrandom(rnd, sizeof(rnd));

    /*
    * Discard early keystream, as per recommendations in:
    * http://www.wisdom.weizmann.ac.il/~itsik/RC4/Papers/Rc4_ksa.ps
    */
    for (i = 0; i < 256; i++)
        (void)arc4_getbyte();
    arc4_count = 1600000;
}

static inline u_int8_t arc4_getbyte(void)
{
    u_int8_t si, sj;

    rs.i = (rs.i + 1);
    si = rs.s[rs.i];
    rs.j = (rs.j + si);
    sj = rs.s[rs.j];
    rs.s[rs.i] = sj;
    rs.s[rs.j] = si;
    return (rs.s[(si + sj) & 0xff]);
}

static inline u_int32_t arc4_getword(void)
{
    u_int32_t val;
    val = arc4_getbyte() << 24;
    val |= arc4_getbyte() << 16;
    val |= arc4_getbyte() << 8;
    val |= arc4_getbyte();
    return val;
}

void arc4random_stir(void)
{
    arc4_stir();
}

void arc4random_addrandom(u_char *dat, int datlen)
{
    if (!rs_initialized)
        arc4_stir();
    arc4_addrandom(dat, datlen);
}

u_int32_t arc4randomT(void)
{
    u_int32_t val;
    arc4_count -= 4;
    val = arc4_getword();
    return val;
}

void arc4random_buf(void *_buf, size_t n)
{
    u_char *buf = (u_char *)_buf;
    while (n--) {
        if (--arc4_count <= 0)
            arc4_stir();
        buf[n] = arc4_getbyte();
    }
}

/*
* Calculate a uniformly distributed random number less than upper_bound
* avoiding "modulo bias".
*
* Uniformity is achieved by generating new random numbers until the one
* returned is outside the range [0, 2**32 % upper_bound).  This
* guarantees the selected random number will be inside
* [2**32 % upper_bound, 2**32) which maps back to [0, upper_bound)
* after reduction modulo upper_bound.
*/
u_int32_t arc4random_uniformT(u_int32_t upper_bound)
{
    u_int32_t r, min;

    if (upper_bound < 2)
        return 0;

    /* 2**32 % x == (2**32 - x) % x */
    min = (unsigned)(-(int)upper_bound) % upper_bound;

    /*
    * This could theoretically loop forever but each retry has
    * p > 0.5 (worst case, usually far better) of selecting a
    * number inside the range we need, so it should rarely need
    * to re-roll.
    */
    for (;;) {
        r = arc4randomT();
        if (r >= min)
            break;
    }

    return r % upper_bound;
}

unsigned int arc4random_uniform(unsigned int upper_bound)
{
    return arc4random_uniformT(upper_bound);
}

unsigned int arc4random()
{
    return arc4randomT();
}

int earc4random()
{
    return arc4random();
}

int earc4random_m(int maxValue)
{
    return arc4random_uniform(maxValue);
}

int earc4random_mm(int minValue, int maxValue)
{
    return arc4random_uniform(maxValue - minValue) + minValue;
}
