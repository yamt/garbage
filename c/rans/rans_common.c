#include "rans_common.h"

prob_t
rans_probs_c(const prob_t ps[NSYMS], sym_t sym)
{
        prob_t c_sym = 0;
        sym_t i;
        for (i = 0; i < sym; i++) {
                c_sym += ps[i];
        }
        return c_sym;
}
