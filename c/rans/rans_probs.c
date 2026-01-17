#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#if defined(RANS_DEBUG)
#include <stdio.h>
#endif

#include "rans_common.h"
#include "rans_probs.h"

static double
calc_bits1(size_t dist, size_t dist_sum, size_t count)
{
        return log2((double)dist_sum / dist) * count;
}

double
calc_bits(const size_t dist[RANS_NSYMS], size_t dist_sum,
          const size_t real_ps[RANS_NSYMS])
{
        double bits_sum = 0;
        unsigned int i;
        for (i = 0; i < RANS_NSYMS; i++) {
                if (real_ps[i] > 0) {
                        assert(dist[i] <= dist_sum);
                        bits_sum += calc_bits1(dist[i], dist_sum, real_ps[i]);
                }
        }
        return bits_sum;
}

size_t
calc_sum(const size_t ps[RANS_NSYMS])
{
        size_t psum = 0;
        unsigned int i;
        for (i = 0; i < RANS_NSYMS; i++) {
                psum += ps[i];
        }
        return psum;
}

void
rans_probs_init(struct rans_probs *ps, const size_t ops[RANS_NSYMS])
{
        /*
         * scale probabilities in ops[] so that:
         *
         * - sum(ps) = M
         * - each p fits prob_t
         * - the distinction between zero and non-zero is preserved
         */
        size_t opsum;
        size_t psum = opsum = calc_sum(ops);
        if (psum == 0) {
                memset(ps->ls, 0, sizeof(ps->ls));
                return;
        }

        size_t ls[RANS_NSYMS];
        unsigned int i;
        for (i = 0; i < RANS_NSYMS; i++) {
                size_t n = ops[i] * RANS_M / psum;
                if (n == 0 && ops[i] != 0) {
                        n = 1;
                }
                if (n > RANS_M - 1) {
                        n = RANS_M - 1;
                }
                ls[i] = n;
        }

        while (1) {
                psum = calc_sum(ls);
                assert(psum > 0);
                assert(psum <= RANS_M);
                if (psum == RANS_M) {
                        break;
                }
                double max_d = -1;
                unsigned int max_i = RANS_NSYMS;
                for (i = 0; i < RANS_NSYMS; i++) {
                        if (ls[i] == 0) {
                                continue;
                        }
                        if (ls[i] == RANS_M - 1) {
                                goto done;
                        }
                        double cur = calc_bits1(ls[i], psum, ops[i]);
                        double cur1 = calc_bits1(ls[i] + 1, psum, ops[i]);
                        assert(cur >= cur1);
                        double d = cur - cur1;
                        if (d > max_d) {
                                max_i = i;
                                max_d = d;
                        }
                }
                assert(max_i != RANS_NSYMS);
                ls[max_i]++;
        }
done:
        assert(calc_sum(ls) == RANS_M || calc_sum(ls) == RANS_M - 1);

        /*
         * copy to ps->ls
         */
        for (i = 0; i < RANS_NSYMS; i++) {
                assert(ls[i] < RANS_M);
                ps->ls[i] = ls[i];
        }

#if defined(RANS_DEBUG)
        double bits = calc_bits(ops, opsum, ops);
        double bits_with_our_dist = calc_bits(ls, RANS_M, ops);
        printf("scaling increased the entropy by %f\n",
               bits_with_our_dist - bits);

        for (i = 0; i < RANS_NSYMS; i++) {
                rans_prob_t l_s = ps->ls[i];
                if (l_s == 0) {
                        continue;
                }
                rans_prob_t b_s = rans_b(ps->ls, i);
                double ent = log2((double)RANS_M / l_s);
                double oent = log2((double)opsum / ops[i]);
                printf("[%3x] l_s=%5u, %5u-%5u Is={%08x,...,%08x} ent %.3f "
                       "(%+.8f)\n",
                       i, l_s, b_s, b_s + l_s - 1, RANS_I_SYM_MIN(l_s),
                       RANS_I_SYM_MAX(l_s), ent, ent - oent);
        }
        printf("                             I ={%08x,...,%08x}\n", RANS_I_MIN,
               RANS_I_MAX);
#endif
}

void
count_syms(size_t counts[RANS_NSYMS], const void *input, size_t inputsize)
{
        const uint8_t *cp = input;
        size_t i;
        for (i = 0; i < inputsize; i++) {
                rans_sym_t sym = cp[i];
                counts[sym]++;
        }
}

void
rans_probs_table_with_trans(const struct rans_probs *ps, rans_prob_t *out,
                            rans_sym_t *trans, size_t *nelemp)
{
        unsigned int nsyms = 0;
        unsigned int i;
        for (i = 0; i < RANS_NSYMS; i++) {
                if (ps->ls[i] > 0) {
                        trans[nsyms] = i;
                        out[nsyms] = ps->ls[i];
                        nsyms++;
                }
        }
        *nelemp = nsyms;
}

void
rans_probs_table(const struct rans_probs *ps, rans_prob_t *out, size_t *nelemp)
{
        unsigned int n = RANS_NSYMS;
        while (true) {
                n--;
                if (ps->ls[n] > 0) {
                        n++;
                        break;
                }
                if (n == 0) {
                        break;
                }
        }
        unsigned int i;
        for (i = 0; i < n; i++) {
                out[i] = ps->ls[i];
        }
        *nelemp = n;
}
