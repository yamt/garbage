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
        size_t pmax = 0;
        sym_t pmax_sym = 0;
        unsigned int i;
        for (i = 0; i < NSYMS; i++) {
                prob_t p = ops[i];
                if (pmax < p) {
                        pmax = p;
                        pmax_sym = i;
                }
        }
        size_t psum = calc_psum(ops);
        assert(psum > 0);
        for (i = 0; i < NSYMS; i++) {
                size_t n = ops[i] * M / psum;
                assert((ops[i] > 0) == (n > 0));
                ops[i] = n;
        }
        psum = calc_psum(ops);
        assert(psum > 0);
        assert(M >= psum);
        ops[pmax_sym] += M - psum;
        if (ops[pmax_sym] == M) { /* avoid prob_t overflow */
                if (pmax_sym == 0) {
                        ops[0]++;
                } else {
                        ops[1]++;
                }
                ops[pmax_sym]--;
        }
        assert(calc_psum(ops) == M);
        for (i = 0; i < NSYMS; i++) {
                assert(ops[i] < M);
                ps->ps[i] = ops[i];
        }

        for (i = 0; i < NSYMS; i++) {
                prob_t p = ps->ps[i];
                if (p == 0) {
                        continue;
                }
                prob_t c = rans_probs_c(ps->ps, i);
#if defined(RANS_DEBUG)
                printf("[%02x] p=%u, %u-%u Is={%08x-%08x}\n", i, p, c,
                       c + p - 1, (I)L / M * p, (I)B * L / M * p - 1);
#endif
        }
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
