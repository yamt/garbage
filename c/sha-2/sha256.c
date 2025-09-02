/*
 * SHA-256
 *
 * https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "table.h"

static uint32_t
rotr(uint32_t v, unsigned int n)
{
        return (v >> n) | (v << (32 - n));
}

static uint32_t
shr(uint32_t v, unsigned int n)
{
        return v >> n;
}

/* 4.1.2 */
static uint32_t
ch(uint32_t x, uint32_t y, uint32_t z)
{
        return (x & y) ^ (~x & z);
}

/* 4.1.2 */
static uint32_t
maj(uint32_t x, uint32_t y, uint32_t z)
{
        return (x & y) ^ (x & z) ^ (y & z);
}

/* 4.1.2 */
static uint32_t
S0(uint32_t x)
{
        return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

/* 4.1.2 */
static uint32_t
S1(uint32_t x)
{
        return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

/* 4.1.2 */
static uint32_t
s0(uint32_t x)
{
        return rotr(x, 7) ^ rotr(x, 18) ^ shr(x, 3);
}

/* 4.1.2 */
static uint32_t
s1(uint32_t x)
{
        return rotr(x, 17) ^ rotr(x, 19) ^ shr(x, 10);
}

static void
init_w(const uint8_t *p, uint32_t w[64])
{
        /* 6.2.2 1. */
        unsigned int i;
        for (i = 0; i < 16; i++) {
                /* big endian */
                w[i] = ((uint32_t)p[i * 4] << 24) |
                       ((uint32_t)p[i * 4 + 1] << 16) |
                       ((uint32_t)p[i * 4 + 2] << 8) | (uint32_t)p[i * 4 + 3];
        }
        for (; i < 64; i++) {
                w[i] = s1(w[i - 2]) + w[i - 7] + s0(w[i - 15]) + w[i - 16];
        }
}

static void
update_h(const uint32_t w[64], uint32_t h[8])
{
        uint32_t t[8]; /* a,b,c,d,e,f,g,h */
        unsigned int i;

        /* 6.2.2 2. */
        for (i = 0; i < 8; i++) {
                t[i] = h[i];
        }

        /* 6.2.2 3. */
        for (i = 0; i < 64; i++) {
                uint32_t t1 =
                        t[7] + S1(t[4]) + ch(t[4], t[5], t[6]) + K[i] + w[i];
                uint32_t t2 = S0(t[0]) + maj(t[0], t[1], t[2]);

                t[7] = t[6];
                t[6] = t[5];
                t[5] = t[4];
                t[4] = t[3] + t1;
                t[3] = t[2];
                t[2] = t[1];
                t[1] = t[0];
                t[0] = t1 + t2;
        }

        /* 6.2.2 4. */
        for (i = 0; i < 8; i++) {
                h[i] += t[i];
        }
}

static void
chunk(const uint8_t p[64], uint32_t h[8])
{
        uint32_t w[64];

        init_w(p, w);
        update_h(w, h);
}

void
sha256(const void *p, size_t len, uint32_t h[8])
{
        uint8_t tmp[64];

        /* 5.3.3 */
        memcpy(h, H, 32);

        /* XXX handle larger input */
        if (len < 64 - 8 - 1) {
                /* 5.1.1 */
                memcpy(tmp, p, len);
                tmp[len] = 0x80;
                memset(tmp + len + 1, 0, 64 - 8 - len - 1);
                uint64_t l = len * 8;
                tmp[64 - 8] = (l >> 56) & 0xff;
                tmp[64 - 7] = (l >> 48) & 0xff;
                tmp[64 - 6] = (l >> 40) & 0xff;
                tmp[64 - 5] = (l >> 32) & 0xff;
                tmp[64 - 4] = (l >> 24) & 0xff;
                tmp[64 - 3] = (l >> 16) & 0xff;
                tmp[64 - 2] = (l >> 8) & 0xff;
                tmp[64 - 1] = (l >> 0) & 0xff;
                chunk(tmp, h);
        }
}

#include <inttypes.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
        uint32_t h[8];
        sha256("abc", 3, h);
        printf("%" PRIx32 "\n", h[0]); /* ba7816bf */
}
