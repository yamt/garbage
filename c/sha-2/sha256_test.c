#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sha256.h"

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

        /* 1000000 "a"s */
        static const char a64[] = "aaaaaaaaaaaaaaaa"
                                  "aaaaaaaaaaaaaaaa"
                                  "aaaaaaaaaaaaaaaa"
                                  "aaaaaaaaaaaaaaaa";
        size_t left = 1000000;
        assert(strlen(a64) == 64);
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

        /* 1073741824 byte data (note: this is larger than UINT32_MAX) */
        static const char abc[] = "abcdefghbcdefghicdefghijdefghijkefghijkl"
                                  "fghijklmghijklmnhijklmno";
        assert(strlen(abc) == 64); /* the following code assumes this */
        time_t start = time(NULL);
        sha256_init(h);
        uint32_t i;
        for (i = 0; i < 16777216; i++) {
                sha256_block(abc, h);
        }
        sha256_tail(a64, 0, 16777216 * 64, h);
        time_t end = time(NULL);
        static const uint32_t h5[8] = {
                0x50e72a0e, 0x26442fe2, 0x552dc393, 0x8ac58658,
                0x228c0cbf, 0xb1d2ca87, 0x2ae43526, 0x6fcd055e,
        };
        check(h, h5);
        printf("%g M byte per sec\n",
               (double)16777216 * 64 / 1024 / 1024 / (end - start));
}
