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
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "base64decode.h"

#if !defined(__has_builtin)
#define __has_builtin(a) 0
#endif

#if !__has_builtin(__builtin_assume)
#define __builtin_assume(cond)
#endif

#if !defined(LITTLE_ENDIAN)
#if defined(__LITTLE_ENDIAN__) || (defined(__BYTE_ORDER__) &&                 \
                                   __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define LITTLE_ENDIAN 1
#endif
#endif

#if !defined(LITTLE_ENDIAN)
#if defined(__BIG_ENDIAN__) ||                                                \
        (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define LITTLE_ENDIAN 0
#endif
#endif

#if !defined(LITTLE_ENDIAN)
#error endian is not known
#endif

#if defined(NDEBUG)
#define BASE64_ASSUME(cond) __builtin_assume(cond)
#else
#define BASE64_ASSUME(cond) assert(cond)
#endif

static int
storebe3(uint8_t dst[3], uint32_t x, unsigned int len)
{
        BASE64_ASSUME(1 <= len && len <= 3);
        BASE64_ASSUME((x & 0xff000000) == 0);
        dst[0] = x >> 16;
        if (len == 1) {
                return (x & 0xffff) != 0;
        }
        dst[1] = x >> 8;
        if (len == 2) {
                return (x & 0xff) != 0;
        }
        dst[2] = x;
        return 0;
}

static uint8_t
conv_from_char(uint8_t x)
{
#define E(a, b)                                                               \
        case a:                                                               \
                return b;
        switch (x) {
#include "base64_chars.h"
        };
        return -1;
}

static uint32_t
shrink(uint32_t x)
{
        BASE64_ASSUME((x & 0x80808080) == 0);
        return ((x & 0x3f000000) >> 6) | ((x & 0x003f0000) >> 4) |
               ((x & 0x00003f00) >> 2) | (x & 0x0000003f);
}

static uint32_t
byteswap(uint32_t x)
{
        return ((x << 24) & 0xff000000) | ((x << 8) & 0x00ff0000) |
               ((x >> 8) & 0x0000ff00) | ((x >> 24) & 0x000000ff);
}

static int
convert_from_chars(uint32_t *dst, const uint8_t p[4], unsigned int *lenp)
{
        union {
                uint32_t u32;
                uint8_t u8[4];
        } u;
        unsigned int len = 3;
        if (p[3] == '=') {
                len = 2;
                if (p[2] == '=') {
                        len = 1;
                }
        }
        u.u32 = 0;
        unsigned int i;
        for (i = 0; i < len + 1; i++) {
                u.u8[i] = conv_from_char(p[i]);
                if (u.u8[i] == (uint8_t)-1) {
                        return -1;
                }
        }
        *lenp = len;
        *dst = u.u32; /* host endian */
        return 0;
}

static int
dec4(const uint8_t p[4], uint8_t dst[3], unsigned int *lenp)
{
        unsigned int len;
        uint32_t x;
        if (convert_from_chars(&x, p, &len)) {
                return -1;
        }
        BASE64_ASSUME((x & 0x80808080) == 0);
#if LITTLE_ENDIAN
        x = byteswap(x);
#endif
        x = shrink(x);
        if (storebe3(dst, x, len)) {
                return -1;
        }
        *lenp = len;
        return 0;
}

/*
 * return large enough buffer size to decode a srclen-bytes base64 string.
 * it can be a few bytes larger than the decoded data.
 */
size_t
base64decode_size(size_t srclen)
{
        return srclen / 4 * 3;
}

size_t
base64decode_size_exact(const void *src, size_t srclen)
{
        size_t sz = base64decode_size(srclen);
        if (sz == 0 || sz / 3 * 4 != srclen) {
                return sz; /* invalid base64 */
        }
        BASE64_ASSUME(srclen >= 4);
        const uint8_t *p = src;
        if (p[srclen - 1] == '=') {
                if (p[srclen - 2] == '=') {
                        return sz - 2;
                }
                return sz - 1;
        }
        return sz;
}

/* return -1 on invalid encoding */
int
base64decode(const void *restrict src, size_t srclen, void *restrict dst,
             size_t *decoded_sizep)
{
        if (srclen % 4 != 0) {
                return -1;
        }
        size_t n = srclen / 4;
        size_t i;

        const uint8_t *sp = src;
        uint8_t *dp = dst;
        for (i = 0; i < n; i++) {
                unsigned int len;
                if (dec4(sp, dp, &len)) {
                        return -1;
                }
                dp += len;
                if (len < 3) {
                        if (i + 1 != n) {
                                return -1;
                        }
                        break;
                }
                sp += 4;
        }
        *decoded_sizep = dp - (uint8_t *)dst;
        return 0;
}

#if defined(TEST)

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void
tests(void)
{
        char dbuf[5];
        size_t dsize;
        memset(dbuf, 'x', 5);
        assert(base64decode("aaaa", 0, dbuf, &dsize) == 0);
        assert(dsize == 0);
        assert(!memcmp(dbuf, "xxxxx", 5));
        assert(base64decode("=", 1, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf, "xxxxx", 5));
        assert(base64decode("==", 2, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf, "xxxxx", 5));
        assert(base64decode("===", 3, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf, "xxxxx", 5));
        assert(base64decode("====", 4, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf, "xxxxx", 5));
        assert(base64decode("aa=a", 4, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf, "xxxxx", 5));

        memset(dbuf, 'x', 5);
        assert(base64decode("aa==", 4, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf + 1, "xxxx", 4));
        memset(dbuf, 'x', 5);
        assert(base64decode("ab==", 4, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf + 1, "xxxx", 4));
        memset(dbuf, 'x', 5);
        assert(base64decode("ac==", 4, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf + 1, "xxxx", 4));
        memset(dbuf, 'x', 5);
        assert(base64decode("ad==", 4, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf + 1, "xxxx", 4));
        memset(dbuf, 'x', 5);
        assert(base64decode("ae==", 4, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf + 1, "xxxx", 4));
        memset(dbuf, 'x', 5);
        assert(base64decode("af==", 4, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf + 1, "xxxx", 4));
        memset(dbuf, 'x', 5);
        assert(base64decode("ag==", 4, dbuf, &dsize) == 0);
        assert(dsize == 1 && !memcmp(dbuf, "\152xxxx", 5));
        memset(dbuf, 'x', 5);
        assert(base64decode("ah==", 4, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf + 1, "xxxx", 4));

        memset(dbuf, 'x', 5);
        assert(base64decode("aaa=", 4, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf + 2, "xxx", 3));
        memset(dbuf, 'x', 5);
        assert(base64decode("aab=", 4, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf + 2, "xxx", 3));
        memset(dbuf, 'x', 5);
        assert(base64decode("aac=", 4, dbuf, &dsize) == 0);
        assert(dsize == 2 && !memcmp(dbuf, "\151\247xxx", 5));
        memset(dbuf, 'x', 5);
        assert(base64decode("aad=", 4, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf + 2, "xxx", 3));

        memset(dbuf, 'x', 5);
        assert(base64decode("YQ==aaaa", 8, dbuf, &dsize) == -1);
        assert(!memcmp(dbuf + 1, "xxxx", 4));
        memset(dbuf, 'x', 5);
        assert(base64decode("YQ==aaaa", 4, dbuf, &dsize) == 0);
        assert(dsize == 1 && !memcmp(dbuf, "axxxx", 5));
        memset(dbuf, 'x', 5);
        assert(base64decode("YWI=aaaa", 4, dbuf, &dsize) == 0);
        assert(dsize == 2 && !memcmp(dbuf, "abxxx", 5));
        memset(dbuf, 'x', 5);
        assert(base64decode("YWJjaaaa", 4, dbuf, &dsize) == 0);
        assert(dsize == 3 && !memcmp(dbuf, "abcxx", 5));

        assert(base64decode_size(0) == 0);
        assert(base64decode_size(1) == 0); /* invalid base64 */
        assert(base64decode_size(2) == 0); /* invalid base64 */
        assert(base64decode_size(3) == 0); /* invalid base64 */
        assert(base64decode_size(4) == 3);
        assert(base64decode_size(5) == 3); /* invalid base64 */
        assert(base64decode_size(6) == 3); /* invalid base64 */
        assert(base64decode_size(7) == 3); /* invalid base64 */
        assert(base64decode_size(8) == 6);

        assert(base64decode_size_exact("", 0) == 0);
        assert(base64decode_size_exact("====", 4) == 1); /* invalid base64 */
        assert(base64decode_size_exact("Y===", 4) == 1); /* invalid base64 */
        assert(base64decode_size_exact("YQ==", 4) == 1);
        assert(base64decode_size_exact("YWI=", 4) == 2);
        assert(base64decode_size_exact("aaa=", 4) == 2); /* invalid base64 */
        assert(base64decode_size_exact("YW=I", 4) == 3); /* invalid base64 */
        assert(base64decode_size_exact("YWJj", 4) == 3);
        assert(base64decode_size_exact("===", 3) == 0); /* invalid base64 */
        assert(base64decode_size_exact("==", 2) == 0);  /* invalid base64 */
        assert(base64decode_size_exact("=", 1) == 0);   /* invalid base64 */
        assert(base64decode_size_exact("YWJjZA==", 8) == 4);
}

int
main(void)
{
        tests();

        while (true) {
                uint8_t buf[4 * 100];
                char dbuf[3 * 100];
                ssize_t ssz = read(STDIN_FILENO, buf, sizeof(buf));
                if (ssz == -1) {
                        fprintf(stderr, "read error\n");
                        exit(1);
                }
                if (ssz == 0) {
                        break;
                }
                size_t dsize;
                int ret = base64decode(buf, ssz, dbuf, &dsize);
                if (ret != 0) {
                        fprintf(stderr, "invalid encoding\n");
                        exit(1);
                }
                write(STDOUT_FILENO, dbuf, dsize);
        }
}

#endif /* defined(TEST) */
