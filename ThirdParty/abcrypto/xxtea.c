#include "xxtea.h"

/*  the useful constant for TEA: (sqrt(5.0) - 1.0)*(2^31) */
const uint32_t tea_delta = 0x9E3779B9;
void xxtea_enc(uint32_t * msg,
    const uint32_t n,
    const uint32_t *const key)
{
    register uint32_t y, z, sum, e, p, q;

    z = msg[n - 1];
    y = msg[0];
    q = 6 + 52 / n;   /* 6 iterations, more if the block is small */
    sum = 0;

    while (q--)
    {
        sum += tea_delta;
        e = (sum >> 2) & 3;

        for (p = 0; p < n - 1; ++p)
        {
            y = msg[p + 1];
            z = msg[p] += ((((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4))) ^
                (sum ^ y)) +
                (key[(p & 3) ^ e] ^ z);
        }

        y = msg[0];
        z = msg[n - 1] += ((((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4))) ^
            (sum ^ y)) +
            (key[(p & 3) ^ e] ^ z);
    }
}

void xxtea_dec(uint32_t * msg,
    const uint32_t n,
    const uint32_t *const key)
{
    register uint32_t y, z, sum, e, p, q;

    q = 6 + 52 / n;   /* 6 iterations, more if the block is small */
    y = msg[0];
    sum = q*tea_delta;

    while (sum)
    {
        e = (sum >> 2) & 3;

        for (p = n - 1; p; --p)
        {
            z = msg[p - 1];
            y = msg[p] -= ((((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4))) ^
                (sum ^ y)) +
                (key[(p & 3) ^ e] ^ z);
        }
        z = msg[n - 1];
        y = msg[0] -= ((((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4))) ^
            (sum ^ y)) +
            (key[(p & 3) ^ e] ^ z);

        sum -= tea_delta;
    }
}
