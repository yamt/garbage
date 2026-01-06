#include <stdint.h>

typedef unsigned int woff_t;
typedef void (*output_literal_t)(void *ctx, uint8_t ch);
typedef void (*output_match_t)(void *ctx, woff_t dist, woff_t len);

#define MATCH_LEN_MIN 3
#define MATCH_LEN_MAX 6
#define MATCH_DISTANCE_MIN 1
#define MATCH_DISTANCE_MAX 64
#define WINDOW_SIZE_MAX MATCH_DISTANCE_MAX
#define LOOKAHEAD_SIZE_MAX MATCH_LEN_MAX
#define BUFSIZE (WINDOW_SIZE_MAX + LOOKAHEAD_SIZE_MAX)

struct lz_encode_state {
        /* output callbacks */
        output_literal_t out_literal;
        output_match_t out_match;
        void *out_ctx;

        /* internal state */
        woff_t bufstart;
        woff_t curoff;     /* relative to bufstart */
        woff_t valid_size; /* relative to bufstart */
        uint8_t buf[BUFSIZE];
};

void lz_encode_init(struct lz_encode_state *state);
void lz_encode(struct lz_encode_state *state, const void *p, size_t len);
void lz_encode_flush(struct lz_encode_state *state);
