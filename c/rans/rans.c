/*
 * reference:
 *   Asymmetric numeral systems: entropy coding combining speed of
 *   Huﬀman coding with compression rate of arithmetic coding
 *   Jarek Duda
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* --- parameters */

typedef uint8_t sym_t;
typedef uint32_t prob_t;
typedef uint32_t I;

#define B 256 /* b in the paper */
#define L (128 * M) /* l in the paper */
#define M 65536 /* m in the paper */

/*
 * I   = {L, ..., L * B - 1}
 * I_s = {L / M * l_s, ..., B * L / M * l_s - 1}
 */

#define NSYMS 256

/* --- byteout */

struct byteout {
	uint8_t *p;
    size_t actual;
    size_t allocated;
};

void
byteout_init(struct byteout *bo, void *p, size_t sz)
{
	bo->p = p;
    bo->actual = 0;
    bo->allocated = sz;
}

void
byteout_write(struct byteout *bo, uint8_t byte)
{
    if (bo->actual >= bo->allocated) {
        bo->actual++;
        return;
    }
    bo->p[bo->actual++] = byte;
}

void
rev_byteout_write(struct byteout *bo, uint8_t byte)
{
    if (bo->actual >= bo->allocated) {
        bo->actual++;
        return;
    }
    bo->p[bo->allocated - (++bo->actual)] = byte;
}

void *
rev_byteout_ptr(const struct byteout *bo)
{
    return &bo->p[bo->allocated - bo->actual];
}

/* --- bytein */

struct bytein {
	const uint8_t *p;
    size_t size;
};

void
bytein_init(struct bytein *bi, const void *p, size_t size)
{
	bi->p = p;
    bi->size = size;
}

uint8_t
bytein_read(struct bytein *bi)
{
	assert(bi->size > 0);
    bi->size--;
    return *bi->p++;
}

/* --- probs */

struct rans_probs {
	prob_t ps[NSYMS]; /* l_s in the paper */
};

static prob_t
calc_c(const struct rans_probs *ps, sym_t sym)
{
        prob_t c_sym = 0;
        sym_t i;
        for (i = 0; i < sym; i++) {
            c_sym += ps->ps[i];
        }
        return c_sym;
}

static size_t
calc_psum(size_t ps[NSYMS])
{
    size_t psum = 0;
	unsigned int i;
    for (i = 0; i < NSYMS; i++) {
        psum += ps[i];
	}
    return psum;
}

void
rans_probs_init(struct rans_probs *ps, size_t ops[NSYMS])
{
    size_t pmax = 0;
    sym_t pmax_sym = 0;
    unsigned int i;
    for (i = 0; i < NSYMS; i++) {
        prob_t p = ops[i];
        if (pmax < p) {
            pmax = p;
            pmax_sym = i;
        }
    }
    size_t psum = calc_psum(ops);
    for (i = 0; i < NSYMS; i++) {
        ops[i] *= M / psum;
	}
    psum = calc_psum(ops);
    assert(M >= psum);
    ops[pmax_sym] += M - psum;
    assert(calc_psum(ops) == M);
    for (i = 0; i < NSYMS; i++) {
        ps->ps[i] = ops[i];
    }

    for (i = 0; i < NSYMS; i++) {
            prob_t p = ps->ps[i];
            if (p == 0) {
                continue;
            }
            prob_t c = calc_c(ps, i);
            printf("[%02x] p=%u, %u-%u Is={%08x-%08x}\n", i, p, c, c+p-1, (I)L / M * p, (I)B * L / M * p - 1);
    }
}

/* --- encode */

struct rans_encode_state {
	I x;
};

void
rans_encode_init(struct rans_encode_state *st)
{
	st->x = L;
}

static void
encode_normalize(struct rans_encode_state *st, sym_t sym, prob_t p_sym, struct byteout *bo)
{
	I i_sym_max = (I)B * L / M * p_sym - 1;
	while (st->x > i_sym_max) {
        uint8_t out = (uint8_t)(st->x % B);
        rev_byteout_write(bo, out);
        I newx = st->x / B;
        printf("enc normalize %08x -> %08x, out: %02x\n", st->x, newx, out);
        st->x = newx;
    }
}

void
rans_encode_sym(struct rans_encode_state *st, sym_t sym, prob_t c_sym, prob_t p_sym, struct byteout *bo)
{
	encode_normalize(st, sym, p_sym, bo);
    I q = st->x / p_sym;
    I r = st->x - q * p_sym;
    I newx = q * M + c_sym + r;
    printf("enc (%08x, %02x) -> %08x (c=%u, p=%u, q=%u, r=%u)\n", st->x, sym, newx, c_sym, p_sym, q, r);
    st->x = newx;
}

/* --- decode */

struct rans_decode_state {
	I x;
};

void
rans_decode_init(struct rans_decode_state *st, I x)
{
	st->x = x;
}

/* s(x) in the paper */
static sym_t
find_sym(const struct rans_probs *ps, I r)
{
	assert(r < M);
	unsigned int i;
    for (i = 0; i < NSYMS - 1; i++) {
        prob_t p = ps->ps[i];
        if (r < p) {
            break;
        }
        r -= p;
    }
    assert(i < NSYMS);
    return i;
}

bool
rans_decode_needs_more(const struct rans_decode_state *st)
{
	return st->x <= L;
}

static void
decode_normalize(struct rans_decode_state *st, struct bytein *bi)
{
	while (st->x < L) {
        uint8_t in = bytein_read(bi);
        I newx = st->x * B + in;
        printf("dec normalize in=%02x, %08x -> %08x\n", in, st->x, newx);
        st->x = newx;
    }
}

sym_t
rans_decode_sym(struct rans_decode_state *st, const struct rans_probs *ps, struct bytein *bi)
{
	I q_x_m = st->x / M;
    I mod_x_m = st->x % M;
	sym_t sym = find_sym(ps, mod_x_m);
    prob_t c_sym = calc_c(ps, sym);
    prob_t p_sym = ps->ps[sym];
    I newx = p_sym * q_x_m + mod_x_m - c_sym;
    printf("dec %08x -> (%08x, %02x) (c=%u, p=%u, x/m=%u, mod(x,m)=%u)\n", st->x, newx, sym, c_sym, p_sym, q_x_m, mod_x_m);
    st->x = newx;
    decode_normalize(st, bi);
    return sym;
}

/* --- test */

static I
test_encode(const void *input, size_t inputsize, const struct rans_probs *ps, struct byteout *bo)
{
	printf("encoding...\n");
    struct rans_encode_state st0;
    struct rans_encode_state *st = &st0;

	rans_encode_init(st);
    size_t i = inputsize;
    while (1) {
        i--;
        uint8_t sym = ((const uint8_t *)input)[i];
        prob_t c_sym = calc_c(ps, sym);
        rans_encode_sym(st, sym, c_sym, ps->ps[sym], bo);
        if (i == 0) {
            break;
        }
	}
    return st->x;
}

static void
test_decode(I x, const void *input, size_t inputsize, const struct rans_probs *ps, struct byteout *bo)
{
	printf("decoding...\n");
    struct rans_decode_state st0;
    struct rans_decode_state *st = &st0;

	struct bytein bi;
    bytein_init(&bi, input, inputsize);

	rans_decode_init(st, x);
    while (1) {
        sym_t sym = rans_decode_sym(st, ps, &bi);
        byteout_write(bo, sym);
        if (rans_decode_needs_more(st) && bi.size == 0) {
            break;
        }
	}
}

void
test(const char *p)
{
	const uint8_t *input = (const void *)p;
    size_t inputsize = strlen(p);

	size_t counts[NSYMS];
    memset(counts, 0, sizeof(counts));
    size_t i;
    for (i = 0; i < inputsize; i++) {
        sym_t sym = input[i];
        counts[sym]++;
    }

	struct rans_probs ps;
    rans_probs_init(&ps, counts);

	uint8_t encoded[100];
	struct byteout bo;
    byteout_init(&bo, encoded, sizeof(encoded));
	I x = test_encode(input, inputsize, &ps, &bo);

	uint8_t decoded[100];
	struct byteout bo_dec;
    byteout_init(&bo_dec, decoded, sizeof(decoded));
	test_decode(x, rev_byteout_ptr(&bo), bo.actual, &ps, &bo_dec);
    assert(bo_dec.actual == inputsize);
    assert(!memcmp(bo_dec.p, input, inputsize));

    printf("decoded correctly\n");
    printf("compression %zu -> %zu\n", inputsize, bo.actual);
}

int
main(void)
{
 	test("this is a pen. i am a pen. you your yours.");
}
