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

/* common logic between SHA-256 and SHA-512 */

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

/* 4.1.2 */
/* 4.1.3 */
static word
ch(word x, word y, word z)
{
        return (x & y) ^ (~x & z);
}

/* 4.1.2 */
/* 4.1.3 */
static word
maj(word x, word y, word z)
{
        return (x & y) ^ (x & z) ^ (y & z);
}

static uint32_t
be32_decode(const uint8_t *p)
{
        return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
               ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

#if WORD_SIZE == 8
static uint64_t
be64_decode(const uint8_t *p)
{
        return ((uint64_t)be32_decode(p) << 32) | be32_decode(p + 4);
}
#endif

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
