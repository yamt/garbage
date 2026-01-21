#if !defined(_RANS_ENCODE_H_)
#define _RANS_ENCODE_H_

#include "rans_param.h"

struct rans_encode_state {
        rans_I x;
};

/*
 * a few alternatives for encoder state initialization
 *
 * - rans_encode_init: initialize with I_MIN
 * - rans_encode_init_with_prob: initialize with the probability of
 *   the first symbol to encode (ie. the last symbol to decode)
 * - rans_encode_init_zero: initialize with 0
 *
 * rans_encode_init_with_prob allows to start from smaller state and
 * thus likely allows a better compression.
 *
 * rans_encode_zero allows even better compression. a downside is
 * that it requires the decoder know the exact size of the encoded bits.
 *
 * note: "better compression" here means up to a few bytes differences.
 * if your data is large enough, maybe the difference is negligible.
 * in that case, just use rans_encode_init as it's simplest to use.
 */
void rans_encode_init(struct rans_encode_state *st);
void rans_encode_init_zero(struct rans_encode_state *st);
void rans_encode_init_with_prob(struct rans_encode_state *st, rans_prob_t l_s);

/*
 * rans_encode_set_extra: add extra value to the state so that the
 * decoder later can extract it with rans_decode_get_extra.
 * this mechanism can be used to compensate the cost to store the final
 * state of the encoding.
 *
 * note: the current implementation is incompatible with
 * rans_encode_init_zero/rans_encode_init_with_prob.
 */
void rans_encode_set_extra(struct rans_encode_state *st, rans_I extra);

struct bitbuf;
void rans_encode_sym(struct rans_encode_state *st, rans_sym_t sym,
                     rans_prob_t b_s, rans_prob_t l_s, struct bitbuf *bo);
void rans_encode_flush(struct rans_encode_state *st, struct bitbuf *bo);

#endif /* !defined(_RANS_ENCODE_H_) */
