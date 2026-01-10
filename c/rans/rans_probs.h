#include <stddef.h>

#include "rans_param.h"

struct rans_probs {
        prob_t ps[NSYMS]; /* l_s in the paper */
};

void count_syms(size_t counts[NSYMS], const void *input, size_t inputsize);
void rans_probs_init(struct rans_probs *ps, size_t ops[NSYMS]);
