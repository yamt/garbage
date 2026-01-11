/*
 * reference:
 *   Asymmetric numeral systems: entropy coding combining speed of
 *   Huﬀman coding with compression rate of arithmetic coding
 *   Jarek Duda
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rans_common.h"
#include "rans_decode.h"
#include "rans_encode.h"
#include "rans_probs.h"

#include "bytein.h"
#include "byteout.h"

#include "test_util.h"

/* --- test */

static void
test_encode(const void *input, size_t inputsize, const struct rans_probs *ps,
            struct byteout *bo)
{
        printf("encoding...\n");
        struct rans_encode_state st0;
        struct rans_encode_state *st = &st0;

        rans_encode_init(st);
        size_t i = inputsize;
        while (1) {
                i--;
                uint8_t sym = ((const uint8_t *)input)[i];
                prob_t c_sym = rans_probs_c(ps->ps, sym);
                rans_encode_sym(st, sym, c_sym, ps->ps[sym], bo);
                if (i == 0) {
                        break;
                }
        }
        rans_encode_flush(st, bo);
}

static void
test_decode(const void *input, size_t inputsize, const prob_t *ps,
            struct byteout *bo)
{
        printf("decoding...\n");
        struct rans_decode_state st0;
        struct rans_decode_state *st = &st0;

        const uint8_t *cp = input;
        const uint8_t *ep = cp + inputsize;

        rans_decode_init(st);
        while (1) {
                sym_t sym = rans_decode_sym(st, ps, &cp);
                byteout_write(bo, sym);
                if (rans_decode_needs_more(st) && cp == ep) {
                        break;
                }
        }
}

void
test(void)
{
        size_t inputsize;
        const uint8_t *input = read_fd(STDIN_FILENO, &inputsize);

        if (inputsize == 0) {
                printf("zero byte input\n");
                exit(0);
        }

        size_t counts[NSYMS];
        memset(counts, 0, sizeof(counts));
        count_syms(counts, input, inputsize);

        struct rans_probs ps;
        rans_probs_init(&ps, counts);

        struct byteout bo;
        byteout_init(&bo);
        test_encode(input, inputsize, &ps, &bo);

        prob_t table[RANS_TABLE_MAX_NELEMS];
        size_t tablesize;
        rans_probs_table(&ps, table, &tablesize);

        struct byteout bo_dec;
        byteout_init(&bo_dec);
        test_decode(rev_byteout_ptr(&bo), bo.actual, table, &bo_dec);
        assert(bo_dec.actual == inputsize);
        assert(!memcmp(bo_dec.p, input, inputsize));

        byteout_clear(&bo);
        byteout_clear(&bo_dec);

        printf("decoded correctly\n");
        printf("compression %zu -> %zu + %zu\n", inputsize, bo.actual,
               tablesize * sizeof(prob_t));
}

int
main(void)
{
        test();
}
