#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if !defined(__has_builtin)
#define __has_builtin(a) 0
#endif

#if !__has_builtin(__builtin_assume)
#define __builtin_assume(cond)
#endif

#define BASE64_ASSERT(cond) __builtin_assume(cond)

static uint8_t
conv_to_char(uint8_t x)
{
        BASE64_ASSERT(x < 64);
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

static void
enc3(const uint8_t p[3], char dst[4], unsigned int srclen)
{
        BASE64_ASSERT(srclen > 0 && srclen <= 3);

        /* load */
        uint32_t x = ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | p[2];

        /* convert to 4 byte */
        x = ((x << 6) & 0x3f000000) | ((x << 4) & 0x003f0000) |
            ((x << 2) & 0x00003f00) | (x & 0x0000003f);

        /* convert to ascii */
        union {
                uint32_t u32;
                uint8_t u8[4];
        } u;
        u.u32 = x;
        unsigned int j;
        for (j = 0; j < 4; j++) {
                u.u8[j] = conv_to_char(u.u8[j]);
        }
        x = u.u32;

        /* bswap and padding */
        u.u8[0] = x >> 24;
        u.u8[1] = x >> 16;
        if (srclen >= 2) {
                u.u8[2] = x >> 8;
        } else {
                u.u8[2] = '=';
        }
        if (srclen == 3) {
                u.u8[3] = x;
        } else {
                u.u8[3] = '=';
        }

        /* store */
        memcpy(dst, &u.u32, 4);
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
        BASE64_ASSERT(0 <= tail && tail < 3);
        if (tail > 0) {
                uint8_t tmp[3];
                memset(tmp, 0, sizeof(tmp));
                memcpy(tmp, p, tail);
                enc3(tmp, dst, tail);
        }
}

#include <stdio.h>
#include <string.h>

int
main(int argc, char **argv)
{
        const char *p = argv[1];
        size_t sz = strlen(p);
        size_t bsz = (sz + 2) / 3 * 4;
        char buf[bsz];
        base64encode(p, sz, buf);
        printf("%.*s\n", (int)bsz, buf);
}
