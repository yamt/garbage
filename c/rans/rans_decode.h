#include <stdbool.h>

#include "rans_param.h"

struct rans_decode_state {
        I x;
};

void rans_decode_init(struct rans_decode_state *st, I x);
bool rans_decode_needs_more(const struct rans_decode_state *st);

struct bytein;
struct rans_probs;
sym_t rans_decode_sym(struct rans_decode_state *st, const struct rans_probs *ps,
                struct bytein *bi);
