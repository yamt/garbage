#include <assert.h>
#if defined(RANS_DEBUG)
#include <stdio.h>
#endif

#include "rans_common.h"
#include "rans_decode.h"

void
rans_decode_init(struct rans_decode_state *st)
{
        st->x = 0;
}

/* s(x) in the paper */
static sym_t
find_sym_and_c(const prob_t ps[NSYMS], I r, prob_t *cp)
{
        assert(r < M);
        prob_t c = 0;
        unsigned int i;
        for (i = 0; i < NSYMS - 1; i++) {
                prob_t p = ps[i];
                if (r < (I)c + p) {
                        break;
                }
                c += p;
        }
        assert(i < NSYMS);
        *cp = c;
        return i;
}

bool
rans_decode_needs_more(const struct rans_decode_state *st)
{
        return st->x <= I_MIN;
}

static void
decode_normalize(struct rans_decode_state *st, const uint8_t **inpp)
{
        while (st->x < I_MIN) {
                uint8_t in = *(*inpp)++;
                I newx = st->x * B + in;
#if defined(RANS_DEBUG)
                printf("dec normalize in=%02x, %08x -> %08x\n", in, st->x,
                       newx);
#endif
                st->x = newx;
        }
        assert(st->x >= I_MIN);
        assert(st->x <= I_MAX);
}

sym_t
rans_decode_sym(struct rans_decode_state *st, const prob_t ps[NSYMS],
                const uint8_t **inpp)
{
        decode_normalize(st, inpp);
        assert(st->x >= I_MIN);
        assert(st->x <= I_MAX);
        I q_x_m = st->x / M;
        I mod_x_m = st->x % M;
        prob_t c_sym;
        sym_t sym = find_sym_and_c(ps, mod_x_m, &c_sym);
        assert(c_sym == rans_probs_c(ps, sym));
        prob_t p_sym = ps[sym];
        I newx = p_sym * q_x_m + mod_x_m - c_sym;
#if defined(RANS_DEBUG)
        printf("dec %08x -> (%08x, %02x) (c=%u, p=%u, x/m=%u, mod(x,m)=%u)\n",
               st->x, newx, sym, c_sym, p_sym, q_x_m, mod_x_m);
#endif
        st->x = newx;
        return sym;
}
