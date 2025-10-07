/*
 * a straightforward implementation of SHA-256
 *
 * https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "sha256.h"
#include "sha256_table.h"

typedef uint32_t word;
#define WORD_SIZE 4
#define BLOCK_SIZE SHA256_BLOCK_SIZE
#define MSG_SIZE_SIZE 8
#define MSG_SCHED_SIZE 64

#include "sha2_common1.h"

/* 4.1.2 */
static word
S0(word x)
{
        return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

/* 4.1.2 */
static word
S1(word x)
{
        return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

/* 4.1.2 */
static word
s0(word x)
{
        return rotr(x, 7) ^ rotr(x, 18) ^ shr(x, 3);
}

/* 4.1.2 */
static word
s1(word x)
{
        return rotr(x, 17) ^ rotr(x, 19) ^ shr(x, 10);
}

#include "sha2_common2.h"

void
sha256_init(uint32_t h[8])
{

        /* 5.3.3 */
        memcpy(h, H, sizeof(H));
}

void
sha256_block(const void *p, word h[8])
{
        word w[64];

        init_w(p, w);
        update_h(w, h);
}

void
sha256_tail(const void *p, size_t len, uint64_t total_len, word h[8])
{
        uint8_t tmp[BLOCK_SIZE];

        /*
         * 5.1.1
         *
         * we append a one bit to the end of the message
         *
         * we pad the message to be a multiple of 64-byte blocks
         *
         * we store 8-byte message length at the end of the last block
         */

        assert(len < BLOCK_SIZE);
        assert(total_len <= SHA256_MAX_MESSAGE_BYTES);
        if (len > BLOCK_SIZE - MSG_SIZE_SIZE - 1) {
                /* no room for message length. needs two blocks */
                memcpy(tmp, p, len);
                tmp[len] = 0x80; /* 1-bit end of messgage + 7-bit padding */
                memset(tmp + len + 1, 0, BLOCK_SIZE - (len + 1)); /* padding */
                len = 0;
                sha256_block(tmp, h);
                /* last block */
                memset(tmp, 0, BLOCK_SIZE - MSG_SIZE_SIZE); /* padding */
        } else {
                /* last block */
                memcpy(tmp, p, len);
                tmp[len] = 0x80; /* 1-bit end of messgage + 7-bit padding */
                memset(tmp + len + 1, 0,
                       BLOCK_SIZE - (len + 1) - MSG_SIZE_SIZE); /* padding */
        }
        be64_encode(&tmp[64 - 8], total_len * 8);
        sha256_block(tmp, h);
}

void
sha256(const void *vp, size_t len, word h[8])
{
        const uint8_t *p = vp;
        const uint64_t total_len = len;

        sha256_init(h);
        while (len >= BLOCK_SIZE) { /* full block */
                sha256_block(p, h);
                p += BLOCK_SIZE;
                len -= BLOCK_SIZE;
        }
        sha256_tail(p, len, total_len, h);
}
