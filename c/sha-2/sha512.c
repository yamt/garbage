/*
 * a straightforward implementation of SHA-512
 *
 * https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "sha512.h"
#include "sha512_table.h"

typedef uint64_t word;
#define BLOCK_SIZE 128
#define MSG_SIZE_SIZE 16
#define MSG_SCHED_SIZE 80

static word
rotr(word v, unsigned int n)
{
        return (v >> n) | (v << (sizeof(word) * 8 - n));
}

static word
shr(word v, unsigned int n)
{
        return v >> n;
}

/* 4.1.3 */
static word
ch(word x, word y, word z)
{
        return (x & y) ^ (~x & z);
}

/* 4.1.3 */
static word
maj(word x, word y, word z)
{
        return (x & y) ^ (x & z) ^ (y & z);
}

/* 4.1.3 */
static word
S0(word x)
{
        return rotr(x, 28) ^ rotr(x, 34) ^ rotr(x, 39);
}

/* 4.1.3 */
static word
S1(word x)
{
        return rotr(x, 14) ^ rotr(x, 18) ^ rotr(x, 41);
}

/* 4.1.3 */
static word
s0(word x)
{
        return rotr(x, 1) ^ rotr(x, 8) ^ shr(x, 7);
}

/* 4.1.3 */
static word
s1(word x)
{
        return rotr(x, 19) ^ rotr(x, 61) ^ shr(x, 6);
}

static uint32_t
be32_decode(const uint8_t *p)
{
        return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
               ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint64_t
be64_decode(const uint8_t *p)
{
        return ((uint64_t)be32_decode(p) << 32) | be32_decode(p + 4);
}

static void
be32_encode(uint8_t *p, uint32_t v)
{
        p[0] = (v >> 24) & 0xff;
        p[1] = (v >> 16) & 0xff;
        p[2] = (v >> 8) & 0xff;
        p[3] = (v >> 0) & 0xff;
}

static void
be64_encode(uint8_t *p, uint64_t v)
{
        be32_encode(p, v >> 32);
        be32_encode(p + 4, v & 0xffffffff);
}

static void
init_w(const uint8_t *p, word w[MSG_SCHED_SIZE])
{
        /* 6.4.2 1. */
        unsigned int i;
        for (i = 0; i < 16; i++) {
                w[i] = be64_decode(&p[i * 8]);
        }
        for (; i < MSG_SCHED_SIZE; i++) {
                w[i] = s1(w[i - 2]) + w[i - 7] + s0(w[i - 15]) + w[i - 16];
        }
}

static void
update_h(const word w[MSG_SCHED_SIZE], word h[8])
{
        word t[8]; /* a,b,c,d,e,f,g,h */
        unsigned int i;

        /* 6.4.2 2. */
        for (i = 0; i < 8; i++) {
                t[i] = h[i];
        }

        /* 6.4.2 3. */
        for (i = 0; i < MSG_SCHED_SIZE; i++) {
                word t1 = t[7] + S1(t[4]) + ch(t[4], t[5], t[6]) + K[i] + w[i];
                word t2 = S0(t[0]) + maj(t[0], t[1], t[2]);

                t[7] = t[6];
                t[6] = t[5];
                t[5] = t[4];
                t[4] = t[3] + t1;
                t[3] = t[2];
                t[2] = t[1];
                t[1] = t[0];
                t[0] = t1 + t2;
        }

        /* 6.4.2 4. */
        for (i = 0; i < 8; i++) {
                h[i] += t[i];
        }
}

void
sha512_init(word h[8])
{

        /* 5.3.5 */
        memcpy(h, H, sizeof(H));
}

void
sha512_block(const void *p, word h[8])
{
        word w[MSG_SCHED_SIZE];

        init_w(p, w);
        update_h(w, h);
}

void
sha512_tail(const void *p, size_t len, uint64_t total_len, word h[8])
{
        uint8_t tmp[BLOCK_SIZE];

        /*
         * 5.1.2
         *
         * we append a one bit to the end of the message
         *
         * we pad the message to be a multiple of 128-byte (1024-bit) blocks
         *
         * we store 16-byte message length at the end of the last block
         */

        assert(len < BLOCK_SIZE);
        if (len > BLOCK_SIZE - MSG_SIZE_SIZE - 1) {
                /* no room for message length. needs two blocks */
                memcpy(tmp, p, len);
                tmp[len] = 0x80; /* 1-bit end of messgage + 7-bit padding */
                memset(tmp + len + 1, 0, BLOCK_SIZE - (len + 1)); /* padding */
                len = 0;
                sha512_block(tmp, h);
                /* last block */
                memset(tmp, 0, BLOCK_SIZE - MSG_SIZE_SIZE); /* padding */
        } else {
                /* last block */
                memcpy(tmp, p, len);
                tmp[len] = 0x80; /* 1-bit end of messgage + 7-bit padding */
                memset(tmp + len + 1, 0,
                       BLOCK_SIZE - (len + 1) - MSG_SIZE_SIZE); /* padding */
        }
        be64_encode(&tmp[BLOCK_SIZE - MSG_SIZE_SIZE], total_len >> (64 - 3));
        be64_encode(&tmp[BLOCK_SIZE - MSG_SIZE_SIZE + 8], total_len << 3);
        sha512_block(tmp, h);
}

void
sha512(const void *vp, size_t len, word h[8])
{
        const uint8_t *p = vp;
        const uint64_t total_len = len;

        sha512_init(h);
        while (len >= BLOCK_SIZE) { /* full block */
                sha512_block(p, h);
                p += BLOCK_SIZE;
                len -= BLOCK_SIZE;
                continue;
        }
        sha512_tail(p, len, total_len, h);
}
