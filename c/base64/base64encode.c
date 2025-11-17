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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "base64encode.h"
#include "base64impl.h"

static uint8_t
conv_to_char(uint8_t x)
{
        BASE64_ASSUME(x < 64);
#define E(a, b) [b] = a,
        static const char table[64] = {
#include "base64chars.h"
        };
        return table[x];
}

static uint32_t
loadbe(const uint8_t p[3])
{
        return ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | p[2];
}

static uint32_t
expand(uint32_t x)
{
        BASE64_ASSUME((x & 0xff000000) == 0);
        uint32_t t = ((x << 6) & 0x3f000000) | ((x << 4) & 0x003f0000) |
                     ((x << 2) & 0x00003f00) | (x & 0x0000003f);
        BASE64_ASSUME((t & 0x80808080) == 0);
        return t;
}

static uint32_t
convert(uint32_t x)
{
        BASE64_ASSUME((x & 0x80808080) == 0);
        union {
                uint32_t u32;
                uint8_t u8[4];
        } u;
        u.u32 = x;
        unsigned int j;
        for (j = 0; j < 4; j++) {
                u.u8[j] = conv_to_char(u.u8[j]);
        }
        return u.u32;
}

static uint32_t
pad(uint32_t x, unsigned int srclen)
{
        BASE64_ASSUME(srclen > 0 && srclen <= 3);
        if (srclen < 3) {
                union {
                        uint32_t u32;
                        uint8_t u8[4];
                } u;
                u.u32 = x;
                if (srclen == 1) {
                        BASE64_ASSUME(u.u8[2] == 'A');
                        u.u8[2] = '=';
                }
                BASE64_ASSUME(u.u8[3] == 'A');
                u.u8[3] = '=';
                x = u.u32;
        }
        return x;
}

static void
enc3(const uint8_t p[3], char dst[4], unsigned int srclen)
{
        BASE64_ASSUME(srclen > 0 && srclen <= 3);

        uint32_t x = loadbe(p);
        x = expand(x);
#if LITTLE_ENDIAN
        x = byteswap(x);
#endif
        x = convert(x);
        x = pad(x, srclen);
        memcpy(dst, &x, 4);
}

size_t
base64encode_size(size_t srclen)
{
        size_t bsz = (srclen + 2) / 3 * 4;
        BASE64_ASSUME(srclen / 3 == bsz / 4 || srclen / 3 + 1 == bsz / 4);
        return bsz;
}

void
base64encode(const void *restrict src, size_t srclen, char *restrict dst)
{
        const uint8_t *p = src;
        size_t n = srclen / 3;
        size_t i;

        for (i = 0; i < n; i++) {
                enc3(p, dst, 3);
                p += 3;
                dst += 4;
        }
        size_t tail = srclen - n * 3;
        BASE64_ASSUME(0 <= tail && tail < 3);
        if (tail > 0) {
                uint8_t tmp[3];
                memset(tmp, 0, sizeof(tmp));
                memcpy(tmp, p, tail);
                enc3(tmp, dst, tail);
        }
}

#if defined(TEST)

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(void)
{
        while (true) {
                uint8_t buf[3 * 100];
                char ebuf[4 * 100];
                ssize_t ssz = read(STDIN_FILENO, buf, sizeof(buf));
                if (ssz == -1) {
                        fprintf(stderr, "read error\n");
                        exit(1);
                }
                if (ssz == 0) {
                        break;
                }
                base64encode(buf, ssz, ebuf);
                write(STDOUT_FILENO, ebuf, base64encode_size(ssz));
        }
}

#endif /* defined(TEST) */
