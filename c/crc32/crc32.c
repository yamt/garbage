#include "crc32.h"

/*
 * CRC-32 (a variant used by png, zip, etc)
 *
 * a dumb implementation w/o table.
 *
 * references:
 * https://en.wikipedia.org/wiki/Cyclic_redundancy_check
 * https://funini.com/kei/math/crc_basic.shtml
 * https://funini.com/kei/math/crc_math.shtml
 */

#define INITXOR UINT32_C(0xffffffff)
#define FINALXOR UINT32_C(0xffffffff)
#define DIVISOR UINT32_C(0xedb88320) /* reversed polynomial */
#define INV UINT32_C(0x5b358fd3)     /* 2^32 * INV === 1 (mod DIVISOR) */
#define CRC0 UINT32_C(0x2144df1c)

static uint32_t
div_1(uint32_t crc)
{
        if ((crc & 1) == 0) {
                crc = (crc >> 1);
        } else {
                crc = (crc >> 1) ^ DIVISOR;
        }
        return crc;
}

static uint32_t
div_8(uint32_t crc)
{
        unsigned int i;
        for (i = 0; i < 8; i++) {
                crc = div_1(crc);
        }
        return crc;
}

static uint32_t
undiv_8(uint32_t crc)
{
        unsigned int i;
        for (i = 0; i < 8; i++) {
                /* note: the msb of DIVISOR is 1 */
                if ((crc & 0x80000000) != 0) {
                        crc ^= DIVISOR;
                        crc <<= 1;
                        crc |= 1;
                } else {
                        crc <<= 1;
                }
        }
        return crc;
}

uint32_t
crc32_init(void)
{
        return INITXOR;
}

uint32_t
crc32_finalize(uint32_t crc)
{
        return crc ^ FINALXOR;
}

uint32_t
crc32_update(uint32_t crc, const void *p, size_t len)
{
        const uint8_t *cp = p;
        const uint8_t *const ep = cp + len;
        while (cp < ep) {
                crc ^= *cp++;
                crc = div_8(crc);
        }
        return crc;
}

uint32_t
crc32(const void *p, size_t len)
{
        uint32_t crc = crc32_init();
        crc = crc32_update(crc, p, len);
        return crc32_finalize(crc);
}

uint32_t
crc32_undo_update(uint32_t crc, const void *p, size_t len)
{
        const uint8_t *const cp = p;
        const uint8_t *ep = cp + len;
        while (cp < ep) {
                crc = undiv_8(crc);
                crc ^= *--ep;
        }
        return crc;
}

uint32_t
crc32_undo_finalize(uint32_t crc)
{
        return crc ^ FINALXOR;
}

/*
 * crc1 = crc32(p1, len1);
 * crc2 = crc32(p2, len2);
 * crc3 = crc32_append(crc1, crc2, len2);
 *
 * crc = crc32_init();
 * crc = crc32_update(crc, p1, len1);
 * crc = crc32_update(crc, p2, len2);
 * crc4 = crc32_finialize(crc);
 *
 * crc3 == crc4
 */
uint32_t
crc32_append(uint32_t crc1, uint32_t crc2, size_t crc2_len)
{
        /*
         * the following crc32_undo_finalize and crc32_init
         * cancel each other.
         */
        uint32_t t = crc32_undo_finalize(crc1);
        t ^= crc32_init();
        size_t i;
        for (i = 0; i < crc2_len; i++) {
                t = div_8(t);
        }
        /*
         * the following crc32_undo_finalize and crc32_finalize
         * cancel each other.
         */
        return crc32_finalize(t ^ crc32_undo_finalize(crc2));
}

/*
 * returns 4 byte data which can be appended to the end of
 * the original data to produce target_crc CRC.
 */
void
crc32_adjust(uint32_t crc, uint32_t target_crc, uint8_t tail[4])
{
        uint32_t target = crc32_undo_finalize(target_crc);
        uint32_t t = 0;
        unsigned int i;
        for (i = 0; i < 32; i++) {
                t = div_1(t);
                if ((target & 1) != 0) {
                        t ^= INV;
                }
                target >>= 1;
        }
        t ^= crc32_undo_finalize(crc);
        tail[0] = t & 0xff;
        tail[1] = (t >> 8) & 0xff;
        tail[2] = (t >> 16) & 0xff;
        tail[3] = (t >> 24) & 0xff;
}

#if defined(TEST)

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

void
test(const void *p, size_t len, uint32_t crc)
{
        assert(crc32(p, len) == crc);
        uint32_t tmp = crc32_update(crc32_init(), p, len);
        assert(crc32_finalize(tmp) == crc);
        assert(crc32_update(tmp, &tmp, 4) == 0); /* little endian assumed */
        assert(crc32_finalize(crc32_update(tmp, &crc, 4)) ==
               CRC0); /* little endian assumed */
        assert(crc32_undo_update(crc32_undo_finalize(crc), p, len) ==
               crc32_init());

        uint32_t tmp2 = crc32_init();
        size_t i;
        for (i = 0; i < len; i++) {
                tmp2 = crc32_update(tmp2, p + i, 1);
        }
        assert(tmp == tmp2);
        assert(crc == crc32_finalize(tmp2));

        uint32_t crc2 = crc32("foobarbaz", 9);
        uint32_t crc3 = crc32_append(crc, crc2, 9);
        assert(crc == crc32_finalize(crc32_undo_update(
                              crc32_undo_finalize(crc3), "foobarbaz", 9)));
        assert(crc3 == crc32_finalize(crc32_update(tmp, "foobarbaz", 9)));

        uint8_t tail[4];
        crc32_adjust(crc, 0x12345678, tail);
        uint32_t crc4 = crc32(tail, 4);
        assert(crc32_append(crc, crc4, 4) == 0x12345678);
}

int
main(int argc, char **argv)
{
        assert(crc32("XYZ", 3) == 0x7d29f8ed);
        assert(crc32("012345", 6) == 0xb86f6b0f);
        assert(crc32("XYZ012345", 9) == 0xebe26378);
        assert(crc32("012345XYZ", 9) == 0x2d7171cd);
        assert(crc32_append(0x7d29f8ed, 0xb86f6b0f, 6) == 0xebe26378);
        assert(crc32_append(0xb86f6b0f, 0x7d29f8ed, 3) == 0x2d7171cd);
        assert(crc32("\0\0\0\0", 4) == CRC0);

        uint32_t t = crc32_init();
        t = crc32_update(t, "message", 7);
        assert(crc32_update(t, &t, 4) == 0); /* little endian assumed */

        t = INV;
        t = div_8(t);
        t = div_8(t);
        t = div_8(t);
        t = div_8(t);
        assert(t == 0x80000000);

        /* zeros following the initial ffffffff don't change the crc value */
        assert(crc32("\377\377\377\377abcd", 8) ==
               crc32("\377\377\377\377\0\0\0\0abcd", 12));

        test("a", 1, 0xe8b7be43);
        test("abcd", 4, 0xed82cd11);
        test("hoge", 4, 0x8b39e45a);
        test("123456789", 9, 0xcbf43926);

        if (argc == 2) {
                printf("%" PRIx32 "\n", crc32(argv[1], strlen(argv[1])));
        }
}
#endif /* defined(TEST) */
