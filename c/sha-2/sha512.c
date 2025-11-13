/*-
 * Copyright (c)2025 YAMAMOTO Takashi,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

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
#define WORD_SIZE 8
#define BLOCK_SIZE SHA512_BLOCK_SIZE
#define MSG_SIZE_SIZE 16
#define MSG_SCHED_SIZE 80

#include "sha2_common1.h"

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

#include "sha2_common2.h"

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
        }
        sha512_tail(p, len, total_len, h);
}
