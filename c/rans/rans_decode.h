/*
 * a note for embedder
 *
 * the minimum set of files necessary for decoding is:
 *   rans_decode.h
 *   rans_decode.c
 *   rans_param.c
 *   rans_override.c
 */

#if !defined(_RANS_DECODE_H_)
#define _RANS_DECODE_H_

#include <stdbool.h>

#include "rans_param.h"

/*
 * decoding process looks like:
 *
 *   encoded stream
 *      |
 *      | RANS_B_BITS bit blocks
 *      | rans_decode_feed
 *      v
 *   rans_decode_state
 *      |
 *      | rans_decode_sym
 *      v
 *   decoded symbols
 */
struct rans_decode_state {
        rans_I x;
};

void rans_decode_init(struct rans_decode_state *st);

/*
 * if rans_decode_need_more returns true and you have more streaming data
 * to feed the decoder, the next RANS_B_BITS bits from the streaming data
 * should be fed with rans_decode_feed before calling rans_decode_sym.
 * this step might be necessary multiple times for a single rans_decode_sym
 * call.
 *
 * if rans_decode_need_more returns true but you've already fed the entire
 * streaming data, just call rans_decode_sym without rans_decode_feed.
 * (it only happens for streams encoded with rans_encode_init_zero. in
 * that case, the decoder performs non-streaming decoding for the rest of
 * symbols.)
 *
 * so, a code fragment to decode a single symbol looks like:
 *
 *   while (rans_decode_need_more(st) && have more streaming data) {
 *       bits = get the next RANS_B_BITS bits of streaming data
 *       rans_decode_feed(st, bits);
 *   }
 *   sym = rans_decode_sym(st, ls);
 *
 * note: unless the streaming data was encoded with rans_encode_init_zero,
 * you can use a simplified version like the following. that is, you don't
 * need to check the end of streaming data as far as you are sure you have
 * the complete streaming data produced with the encoder.
 *
 *   while (rans_decode_need_more(st)) {
 *       bits = get the next RANS_B_BITS bits of streaming data
 *       rans_decode_feed(st, bits);
 *   }
 *   sym = rans_decode_sym(st, ls);
 */
bool rans_decode_need_more(const struct rans_decode_state *st);
void rans_decode_feed(struct rans_decode_state *st, uint16_t input);

/*
 * rans_decode_sym: decode a symbol.
 *
 * the 'ls' array should be identical to rans_probs_t::ls used for encoding.
 */
rans_sym_t rans_decode_sym(struct rans_decode_state *st,
                           const rans_prob_t ls[RANS_NSYMS]);
rans_I rans_decode_get_extra(struct rans_decode_state *st);

#endif /* defined(_RANS_DECODE_H_) */
