#include "rans_encode.h"
#include "byteout.h"

void
rans_encode_init(struct rans_encode_state *st)
{
        st->x = L;
}

static void
encode_normalize(struct rans_encode_state *st, sym_t sym, prob_t p_sym,
                 struct byteout *bo)
{
        I i_sym_max = (I)B * L / M * p_sym - 1;
        while (st->x > i_sym_max) {
                uint8_t out = (uint8_t)(st->x % B);
                rev_byteout_write(bo, out);
                I newx = st->x / B;
#if defined(RANS_DEBUG)
                printf("enc normalize %08x -> %08x, out: %02x\n", st->x, newx,
                       out);
#endif
                st->x = newx;
        }
}

void
rans_encode_sym(struct rans_encode_state *st, sym_t sym, prob_t c_sym,
                prob_t p_sym, struct byteout *bo)
{
        encode_normalize(st, sym, p_sym, bo);
        I q = st->x / p_sym;
        I r = st->x - q * p_sym;
        I newx = q * M + c_sym + r;
#if defined(RANS_DEBUG)
        printf("enc (%08x, %02x) -> %08x (c=%u, p=%u, q=%u, r=%u)\n", st->x,
               sym, newx, c_sym, p_sym, q, r);
#endif
        st->x = newx;
}
