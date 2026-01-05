#include <stdint.h>

typedef unsigned int woff_t;
typedef void (*output_literal_t)(void *ctx, uint8_t ch);
typedef void (*output_match_t)(void *ctx, woff_t dist, woff_t len);

struct lz_encode_state {
        woff_t bufstart;
        woff_t curoff;     /* relative to bufstart */
        woff_t valid_size; /* relative to bufstart */

        output_literal_t out_literal;
        output_match_t out_match;
        void *out_ctx;
};

void lz_encode(struct lz_encode_state *state, const void *p, size_t len);
void lz_encode_flush(struct lz_encode_state *state);
