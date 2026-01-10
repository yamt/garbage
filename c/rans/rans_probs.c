#include <assert.h>
#include <stddef.h>

#include "rans_probs.h"

prob_t
rans_probs_c(const struct rans_probs *ps, sym_t sym)
{
        prob_t c_sym = 0;
        sym_t i;
        for (i = 0; i < sym; i++) {
                c_sym += ps->ps[i];
        }
        return c_sym;
}

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
        for (i = 0; i < NSYMS; i++) {
                ops[i] *= M / psum;
        }
        psum = calc_psum(ops);
        assert(M >= psum);
        ops[pmax_sym] += M - psum;
        assert(calc_psum(ops) == M);
        for (i = 0; i < NSYMS; i++) {
                ps->ps[i] = ops[i];
        }

        for (i = 0; i < NSYMS; i++) {
                prob_t p = ps->ps[i];
                if (p == 0) {
                        continue;
                }
                prob_t c = rans_probs_c(ps, i);
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
