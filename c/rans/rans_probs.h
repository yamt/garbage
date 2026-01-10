#include <stddef.h>

#include "rans_param.h"

struct rans_probs {
        prob_t ps[NSYMS]; /* l_s in the paper */
};

void rans_probs_init(struct rans_probs *ps, size_t ops[NSYMS]);
prob_t rans_probs_c(const prob_t ps[NSYMS], sym_t sym);

void count_syms(size_t counts[NSYMS], const void *input, size_t inputsize);
