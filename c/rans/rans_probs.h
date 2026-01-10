#include <stddef.h>

#include "rans_param.h"

struct rans_probs {
        prob_t ps[NSYMS]; /* l_s in the paper */
};

void rans_probs_init(struct rans_probs *ps, size_t ops[NSYMS]);
prob_t rans_probs_c(const struct rans_probs *ps, sym_t sym);
