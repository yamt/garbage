#include <assert.h>

#include "bytein.h"
#include "rans_decode.h"
#include "rans_probs.h"

void
rans_decode_init(struct rans_decode_state *st, I x)
{
        st->x = x;
}

/* s(x) in the paper */
static sym_t
find_sym(const struct rans_probs *ps, I r)
{
        assert(r < M);
        unsigned int i;
        for (i = 0; i < NSYMS - 1; i++) {
                prob_t p = ps->ps[i];
                if (r < p) {
                        break;
                }
                r -= p;
        }
        assert(i < NSYMS);
        return i;
}

bool
rans_decode_needs_more(const struct rans_decode_state *st)
{
        return st->x <= L;
}

static void
decode_normalize(struct rans_decode_state *st, struct bytein *bi)
{
        while (st->x < L) {
                uint8_t in = bytein_read(bi);
                I newx = st->x * B + in;
#if defined(RANS_DEBUG)
                printf("dec normalize in=%02x, %08x -> %08x\n", in, st->x,
                       newx);
#endif
                st->x = newx;
        }
}

sym_t
rans_decode_sym(struct rans_decode_state *st, const struct rans_probs *ps,
                struct bytein *bi)
{
        I q_x_m = st->x / M;
        I mod_x_m = st->x % M;
        sym_t sym = find_sym(ps, mod_x_m);
        prob_t c_sym = rans_probs_c(ps, sym);
        prob_t p_sym = ps->ps[sym];
        I newx = p_sym * q_x_m + mod_x_m - c_sym;
#if defined(RANS_DEBUG)
        printf("dec %08x -> (%08x, %02x) (c=%u, p=%u, x/m=%u, mod(x,m)=%u)\n",
               st->x, newx, sym, c_sym, p_sym, q_x_m, mod_x_m);
#endif
        st->x = newx;
        decode_normalize(st, bi);
        return sym;
}
