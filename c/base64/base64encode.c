#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "base64encode.h"

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

static uint8_t
conv_to_char(uint8_t x)
{
        BASE64_ASSUME(x < 64);
        static const char table[64] = {
                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
                'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
                'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
                'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2',
                '3', '4', '5', '6', '7', '8', '9', '+', '/',
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
        return ((x << 6) & 0x3f000000) | ((x << 4) & 0x003f0000) |
               ((x << 2) & 0x00003f00) | (x & 0x0000003f);
}

static uint32_t
byteswap(uint32_t x)
{
        return ((x << 24) & 0xff000000) | ((x << 8) & 0x00ff0000) |
               ((x >> 8) & 0x0000ff00) | ((x >> 24) & 0x000000ff);
}

static uint32_t
convert(uint32_t x)
{
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
        union {
                uint32_t u32;
                uint8_t u8[4];
        } u;
        u.u32 = x;
        if (srclen < 3) {
                if (srclen == 1) {
                        u.u8[2] = '=';
                }
                u.u8[3] = '=';
        }
        return u.u32;
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
