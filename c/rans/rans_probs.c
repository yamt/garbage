#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#if defined(RANS_DEBUG)
#include <stdio.h>
#endif

#include "rans_common.h"
#include "rans_probs.h"

static size_t
calc_psum(size_t ps[NSYMS])
{
        size_t psum = 0;
        unsigned int i;
        for (i = 0; i < NSYMS; i++) {
                psum += ps[i];
        }
        return psum;
}

void
rans_probs_init(struct rans_probs *ps, size_t ops[NSYMS])
{
        /*
         * scale probabilities in ops[] so that:
         *
         * - sum(ps) = M
         * - each p fits prob_t
         * - the distinction between zero and non-zero is preserved
         */
        size_t psum = calc_psum(ops);
        assert(psum > 0);

        unsigned int i;
        for (i = 0; i < NSYMS; i++) {
                size_t n = (ops[i] * M + psum - 1) / psum;
                assert((ops[i] > 0) == (n > 0));
                if (n == M) {
                        n = M - 1;
                }
                assert(n < M);
                ops[i] = n;
                assert(ops[i] < M);
        }

        psum = calc_psum(ops);
        assert(psum > 0);
        if (psum != M) {
                int diff = M - psum;
                i = 0;
                while (diff > 0) {
                        if (ops[i] == 0 && ops[i] < M - 1) {
                                ops[i]++;
                                diff--;
                        }
                        i = (i + 1) % NSYMS;
                }
                while (diff < 0) {
                        if (ops[i] != 0 && ops[i] > 1) {
                                ops[i]--;
                                diff++;
                        }
                        i = (i + 1) % NSYMS;
                }
        }
        assert(calc_psum(ops) == M);

        for (i = 0; i < NSYMS; i++) {
                assert(ops[i] < M);
                ps->ps[i] = ops[i];
        }

        /*
         * copy to ps->ps
         */
        for (i = 0; i < NSYMS; i++) {
                prob_t p = ps->ps[i];
                if (p == 0) {
                        continue;
                }
                prob_t c = rans_probs_c(ps->ps, i);
#if defined(RANS_DEBUG)
                printf("[%02x] p=%u, %u-%u Is={%08x,...,%08x}\n", i, p, c,
                       c + p - 1, I_SYM_MIN(p), I_SYM_MAX(p));
#endif
        }
#if defined(RANS_DEBUG)
        printf("I={%08x,...,%08x}\n", I_MIN, I_MAX);
#endif
}

void
count_syms(size_t counts[NSYMS], const void *input, size_t inputsize)
{
        const uint8_t *cp = input;
        size_t i;
        for (i = 0; i < inputsize; i++) {
                sym_t sym = cp[i];
                counts[sym]++;
        }
}

void
rans_probs_table(const struct rans_probs *ps, prob_t *out, size_t *nelemp)
{
        unsigned int n = NSYMS;
        while (true) {
                n--;
                if (ps->ps[n] > 0) {
                        n++;
                        break;
                }
                if (n == 0) {
                        break;
                }
        }
        unsigned int i;
        for (i = 0; i < n; i++) {
                out[i] = ps->ps[i];
        }
        *nelemp = n;
}
