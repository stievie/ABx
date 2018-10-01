#include "sha256.h"

#include <string.h>

#define H0         0x6a09e667
#define H1         0xbb67ae85
#define H2         0x3c6ef372
#define H3         0xa54ff53a
#define H4         0x510e527f
#define H5         0x9b05688c
#define H6         0x1f83d9ab
#define H7         0x5be0cd19

#define Ch(x,y,z)	((x & y) ^ (~x & z))
#define Maj(x,y,z)	((x & y) ^ (x & z) ^ (y & z))
#define Rotr(x,y)	((x >> y) | (x << (32 - y)))
#define e0(x)		(Rotr(x,2) ^ Rotr(x,13) ^ Rotr(x,22))
#define e1(x)		(Rotr(x,6) ^ Rotr(x,11) ^ Rotr(x,25))
#define s0(x)		(Rotr(x,7) ^ Rotr(x,18) ^ (x >> 3))
#define s1(x)       (Rotr(x,17) ^ Rotr(x,19) ^ (x >> 10))

unsigned int K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void sha256_init(sha256_ctx* context)
{
    context->hash[0] = H0;
    context->hash[1] = H1;
    context->hash[2] = H2;
    context->hash[3] = H3;
    context->hash[4] = H4;
    context->hash[5] = H5;
    context->hash[6] = H6;
    context->hash[7] = H7;
    memset(context->data, 0, sizeof(context->data));
    context->total_len = 0;
    context->offset = 0;
}

static void block(sha256_ctx* context)
{
    unsigned int W[64];
    unsigned int a, b, c, d, e, f, g, h;
    unsigned int t1, t2;
    int t;

    for (t = 0; t < 16; ++t)
    {
        W[t] = (((unsigned int)(context->data[t * 4 + 0])) << 24) |
            (((unsigned int)(context->data[t * 4 + 1])) << 16) |
            (((unsigned int)(context->data[t * 4 + 2])) << 8) |
            ((unsigned int)(context->data[t * 4 + 3]));
    }
    for (t = 16; t < 64; ++t)
    {
        W[t] = (s1(W[t - 2]) + W[t - 7] + s0(W[t - 15]) + W[t - 16]);
    }

    a = context->hash[0];
    b = context->hash[1];
    c = context->hash[2];
    d = context->hash[3];
    e = context->hash[4];
    f = context->hash[5];
    g = context->hash[6];
    h = context->hash[7];

    for (t = 0; t < 64; ++t)
    {
        t1 = h + e1(e) + Ch(e, f, g) + K[t] + W[t];
        t2 = e0(a) + Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    context->hash[0] += a;
    context->hash[1] += b;
    context->hash[2] += c;
    context->hash[3] += d;
    context->hash[4] += e;
    context->hash[5] += f;
    context->hash[6] += g;
    context->hash[7] += h;
}

void sha256_update(sha256_ctx* context, const unsigned char *buf, unsigned len)
{
    context->total_len += len;
    int cont = len;
    int tamanho;
    while (cont > 0)
    {
        tamanho = context->offset + cont;

        if (tamanho >= 64)
        {
            memcpy(&context->data[context->offset], &buf[len - cont], (64 - (context->offset)));
            block(context);
            cont = cont - (64 - (context->offset));
            context->offset = 0;
            tamanho = 0;
        }
        else
        {
            memcpy(&context->data[context->offset], &buf[len - cont], cont);
            context->offset += cont;
            cont = 0;
        }
    }
}

void sha256_final(sha256_ctx* context, unsigned char hash[32])
{
    unsigned long long totalBits;
    int i = 56;
    int j = 0;
    if (context->offset < 56)
    {
        context->data[context->offset] = (unsigned char)128;
        context->offset++;
    }
    else
    {
        context->data[context->offset] = (unsigned char)128;
        context->offset++;
        while (context->offset < 64)
        {
            context->data[context->offset] = (unsigned char)0;
            context->offset++;
        }
        block(context);
        context->offset = 0;
    }
    while (context->offset < 56)
    {
        context->data[context->offset] = (unsigned char)0;
        context->offset++;
    }

    totalBits = context->total_len << 3;

    for (; i >= 0; i = i - 8)
    {
        context->data[context->offset] = (unsigned char)totalBits >> i;
        context->offset++;
    }

    block(context);

    i = 0;
    for (; j<8; j++)
    {
        hash[i++] = context->hash[j] >> 24;
        hash[i++] = context->hash[j] >> 16;
        hash[i++] = context->hash[j] >> 8;
        hash[i++] = context->hash[j];
    }
}

void sha256(const char* str, int len, char* hash_out)
{
    sha256_ctx ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, str, len);
    sha256_final(&ctx, hash_out);
}
