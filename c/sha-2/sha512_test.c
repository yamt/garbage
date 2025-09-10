#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sha512.h"

typedef uint64_t word;
#define BLOCK_SIZE SHA512_BLOCK_SIZE

static void
check1(const word a[8], const word b[8], unsigned int line)
{
        unsigned int i;
        for (i = 0; i < 8; i++) {
                if (a[i] != b[i]) {
                        printf("line %u [%u] %" PRIx64 " != %" PRIx64 "\n",
                               line, i, a[i], b[i]);
                        abort();
                }
        }
}

#define check(a, b) check1(a, b, __LINE__)

int
main(int argc, char **argv)
{
        word h[8];

        /*
         * test vectors from https://www.di-mgt.com.au/sha_testvectors.html
         */

        sha512("abc", 3, h);
        static const word h1[8] = {
                0xddaf35a193617aba, 0xcc417349ae204131, 0x12e6fa4e89a97ea2,
                0x0a9eeee64b55d39a, 0x2192992a274fc1a8, 0x36ba3c23a3feebbd,
                0x454d4423643ce80e, 0x2a9ac94fa54ca49f,
        };
        check(h, h1);

        sha512("", 0, h);
        static const word h2[8] = {
                0xcf83e1357eefb8bd, 0xf1542850d66d8007, 0xd620e4050b5715dc,
                0x83f4a921d36ce9ce, 0x47d0d13c5d85f2b0, 0xff8318d2877eec2f,
                0x63b931bd47417a81, 0xa538327af927da3e,
        };
        check(h, h2);

        sha512("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
               448 / 8, h);
        static const word h3[8] = {
                0x204a8fc6dda82f0a, 0x0ced7beb8e08a416, 0x57c16ef468b228a8,
                0x279be331a703c335, 0x96fd15c13b1b07f9, 0xaa1d3bea57789ca0,
                0x31ad85c7a71dd703, 0x54ec631238ca3445,
        };
        check(h, h3);

        sha512("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklm"
               "noijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
               896 / 8, h);
        static const word h6[8] = {
                0x8e959b75dae313da, 0x8cf4f72814fc143f, 0x8f7779c6eb9f7fa1,
                0x7299aeadb6889018, 0x501d289e4900f7e4, 0x331b99dec4b5433a,
                0xc7d329eeb6dd2654, 0x5e96e55b874be909,
        };
        check(h, h6);

        /* 1000000 "a"s */
        static const char a128[] = "aaaaaaaaaaaaaaaa"
                                   "aaaaaaaaaaaaaaaa"
                                   "aaaaaaaaaaaaaaaa"
                                   "aaaaaaaaaaaaaaaa"
                                   "aaaaaaaaaaaaaaaa"
                                   "aaaaaaaaaaaaaaaa"
                                   "aaaaaaaaaaaaaaaa"
                                   "aaaaaaaaaaaaaaaa";
        size_t left = 1000000;
        assert(strlen(a128) == BLOCK_SIZE);
        sha512_init(h);
        while (left >= BLOCK_SIZE) {
                sha512_block(a128, h);
                left -= BLOCK_SIZE;
        }
        sha512_tail(a128, left, 1000000, h);
        static const word h4[8] = {
                0xe718483d0ce76964, 0x4e2e42c7bc15b463, 0x8e1f98b13b204428,
                0x5632a803afa973eb, 0xde0ff244877ea60a, 0x4cb0432ce577c31b,
                0xeb009c5c2c49aa2e, 0x4eadb217ad8cc09b,
        };
        check(h, h4);

        /* 1073741824 byte data (note: this is larger than UINT32_MAX) */
        static const char abc2[] = "abcdefghbcdefghicdefghijdefghijkefghijkl"
                                   "fghijklmghijklmnhijklmno"
                                   "abcdefghbcdefghicdefghijdefghijkefghijkl"
                                   "fghijklmghijklmnhijklmno";
        assert(strlen(abc2) ==
               BLOCK_SIZE); /* the following code assumes this */
        assert(BLOCK_SIZE == 64 * 2);
        time_t start = time(NULL);
        sha512_init(h);
        uint32_t i;
        for (i = 0; i < 16777216 / 2; i++) {
                sha512_block(abc2, h);
        }
        sha512_tail(NULL, 0, 16777216 * 64, h);
        time_t end = time(NULL);
        static const word h5[8] = {
                0xb47c933421ea2db1, 0x49ad6e10fce6c7f9, 0x3d0752380180ffd7,
                0xf4629a712134831d, 0x77be6091b819ed35, 0x2c2967a2e2d4fa50,
                0x50723c9630691f1a, 0x05a7281dbe6c1086,
        };
        check(h, h5);
        printf("%g M byte per sec\n",
               (double)16777216 * 64 / 1024 / 1024 / (end - start));
}
