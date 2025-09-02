/*
 * SHA-256
 *
 * https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf
 */

#include <assert.h>
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

void
sha256_init(uint32_t h[8])
{

        /* 5.3.3 */
        memcpy(h, H, 32);
}

void
sha256_block(const void *p, uint32_t h[8])
{
        uint32_t w[64];

        init_w(p, w);
        update_h(w, h);
}

void
sha256_tail(const void *p, size_t len, size_t total_len, uint32_t h[8])
{
        uint8_t tmp[64];

        assert(len < 64);
        if (len > 64 - 8 - 1) {
                /* no room for message length. needs two blocks */
                memcpy(tmp, p, len);
                tmp[len] = 0x80; /* 1-bit end of messgage + 7-bit padding */
                memset(tmp + len + 1, 0, 64 - (len + 1)); /* padding */
                len = 0;
                sha256_block(tmp, h);
                /* last block */
                memset(tmp, 0, 64 - 8); /* padding */
        } else {
                /* last block */
                memcpy(tmp, p, len);
                tmp[len] = 0x80; /* 1-bit end of messgage + 7-bit padding */
                memset(tmp + len + 1, 0, 64 - (len + 1) - 8); /* padding */
        }
        uint64_t bitlen = total_len * 8;
        tmp[64 - 8] = (bitlen >> 56) & 0xff;
        tmp[64 - 7] = (bitlen >> 48) & 0xff;
        tmp[64 - 6] = (bitlen >> 40) & 0xff;
        tmp[64 - 5] = (bitlen >> 32) & 0xff;
        tmp[64 - 4] = (bitlen >> 24) & 0xff;
        tmp[64 - 3] = (bitlen >> 16) & 0xff;
        tmp[64 - 2] = (bitlen >> 8) & 0xff;
        tmp[64 - 1] = (bitlen >> 0) & 0xff;
        sha256_block(tmp, h);
}

void
sha256(const void *vp, size_t len, uint32_t h[8])
{
        const uint8_t *p = vp;
        const size_t total_len = len;

        sha256_init(h);

        /*
         * 5.1.1
         *
         * we append a one bit to the end of the message
         *
         * we pad the message to be a multiple of 64-byte blocks
         *
         * we store 8-byte message length at the end of the last block
         */
        while (len >= 64) { /* full block */
                sha256_block(p, h);
                p += 64;
                len -= 64;
                continue;
        }
        sha256_tail(p, len, total_len, h);
}

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
check1(const uint32_t a[8], const uint32_t b[8], unsigned int line)
{
        unsigned int i;
        for (i = 0; i < 8; i++) {
                if (a[i] != b[i]) {
                        printf("line %u [%u] %" PRIx32 " != %" PRIx32 "\n",
                               line, i, a[i], b[i]);
                        abort();
                }
        }
}

#define check(a, b) check1(a, b, __LINE__)

int
main(int argc, char **argv)
{
        uint32_t h[8];

        /*
         * test vectors from https://www.di-mgt.com.au/sha_testvectors.html
         */

        sha256("abc", 3, h);
        static const uint32_t h1[8] = {
                0xba7816bf, 0x8f01cfea, 0x414140de, 0x5dae2223,
                0xb00361a3, 0x96177a9c, 0xb410ff61, 0xf20015ad,
        };
        check(h, h1);

        sha256("", 0, h);
        static const uint32_t h2[8] = {
                0xe3b0c442, 0x98fc1c14, 0x9afbf4c8, 0x996fb924,
                0x27ae41e4, 0x649b934c, 0xa495991b, 0x7852b855,
        };
        check(h, h2);

        sha256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
               448 / 8, h);
        static const uint32_t h3[8] = {
                0x248d6a61, 0xd20638b8, 0xe5c02693, 0x0c3e6039,
                0xa33ce459, 0x64ff2167, 0xf6ecedd4, 0x19db06c1,
        };
        check(h, h3);

        static const char a64[] = "aaaaaaaaaaaaaaaa"
                                  "aaaaaaaaaaaaaaaa"
                                  "aaaaaaaaaaaaaaaa"
                                  "aaaaaaaaaaaaaaaa"
                                  "aaaaaaaaaaaaaaaa"
                                  "aaaaaaaaaaaaaaaa"
                                  "aaaaaaaaaaaaaaaa"
                                  "aaaaaaaaaaaaaaaa";
        size_t left = 1000000;
        sha256_init(h);
        while (left >= 64) {
                sha256_block(a64, h);
                left -= 64;
        }
        sha256_tail(a64, left, 1000000, h);
        static const uint32_t h4[8] = {
                0xcdc76e5c, 0x9914fb92, 0x81a1c7e2, 0x84d73e67,
                0xf1809a48, 0xa497200e, 0x046d39cc, 0xc7112cd0,
        };
        check(h, h4);

        static const char abc[] = "abcdefghbcdefghicdefghijdefghijkefghijkl"
                                  "fghijklmghijklmnhijklmno";
        assert(strlen(abc) == 64); /* the following code assumes this */
        sha256_init(h);
        uint32_t i;
        for (i = 0; i < 16777216; i++) {
                sha256_block(abc, h);
        }
        sha256_tail(a64, 0, 16777216 * 64, h);
        static const uint32_t h5[8] = {
                0x50e72a0e, 0x26442fe2, 0x552dc393, 0x8ac58658,
                0x228c0cbf, 0xb1d2ca87, 0x2ae43526, 0x6fcd055e,
        };
        check(h, h5);
}
