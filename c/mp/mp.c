/*
 * https://members.loria.fr/PZimmermann/mca/mca-cup-0.5.9.pdf
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mp.h"

#define ctassert(a) _Static_assert(a, #a)

#if defined(BASE)
ctassert(COEFF_MAX == BASE - 1);
#endif

static const struct bigint zero = {
        .n = 0,
};
static const struct bigint one = {
        .n = 1,
        .d =
                (coeff_t[]){
                        1,
                },
};
static const struct bigint base = {
        .n = 2,
        .d =
                (coeff_t[]){
                        0,
                        1,
                },
};
#if COEFF_MAX == 9
static const struct bigint ten = {
        .n = 2,
        .d =
                (coeff_t[]){
                        0,
                        1,
                },
};
#endif
#if COEFF_MAX >= 10
static const struct bigint ten = {
        .n = 1,
        .d =
                (coeff_t[]){
                        10,
                },
};
#endif

static coeff_t
dig(const struct bigint *a, unsigned int i)
{
        if (i < a->n) {
                return a->d[i];
        }
        return 0;
}

static coeff_t
coeff_addc(coeff_t a, coeff_t b, coeff_t carry_in, coeff_t *carry_out)
{
#if COEFF_MAX == COEFF_TYPE_MAX
        coeff_t t;
        coeff_t carry = 0;
        if (__builtin_add_overflow(a, b, &t)) {
                carry = 1;
        }
        if (__builtin_add_overflow(t, carry_in, &t)) {
                carry++;
        }
        *carry_out = carry;
        return t;
#else
        ctassert(COEFF_MAX < COEFF_TYPE_MAX);
        ctassert(COEFF_MAX < COEFF_TYPE_MAX / 2);
        ctassert(COEFF_MAX * 2 < COEFF_TYPE_MAX);
        assert(0 <= carry_in);
        assert(carry_in <= 1);
        coeff_t c = a + b + carry_in;
        *carry_out = c > COEFF_MAX;
        assert(0 <= *carry_out);
        assert(*carry_out <= 1);
        return c % BASE;
#endif
}

static coeff_t
coeff_subc(coeff_t a, coeff_t b, coeff_t carry_in, coeff_t *carry_out)
{
#if COEFF_MAX == COEFF_TYPE_MAX
        coeff_t t;
        coeff_t carry = 0;
        if (__builtin_sub_overflow(a, b, &t)) {
                carry = 1;
        }
        if (__builtin_sub_overflow(t, carry_in, &t)) {
                carry = 1;
        }
        *carry_out = carry;
        return t;
#else
        assert(0 <= carry_in);
        assert(carry_in <= 1);
        coeff_t c = a - b - carry_in;
        if (c < 0) {
                *carry_out = 1;
                return c + BASE;
        }
        *carry_out = 0;
        return c;
#endif
}

static coeff_t
coeff_mul(coeff_t *highp, coeff_t a, coeff_t b, coeff_t carry_in)
{
#if UINTMAX_MAX / COEFF_MAX >= COEFF_MAX
        ctassert(UINTMAX_MAX / COEFF_MAX >= COEFF_MAX);
        assert(a <= COEFF_MAX);
        assert(b <= COEFF_MAX);
        assert(carry_in <= COEFF_MAX);
        uintmax_t prod = (uintmax_t)a * b + carry_in;
        *highp = prod / BASE;
        return prod % BASE;
#else
        /* revisit: use mulq on x86 */
        ctassert(COEFF_BITS / 2 * 2 == COEFF_BITS);
        const unsigned int hbits = COEFF_BITS / 2;
        const unsigned int hmask = ((coeff_t)1 << hbits) - 1;
        coeff_t a_high = a >> hbits;
        coeff_t a_low = a & hmask;
        coeff_t b_high = b >> hbits;
        coeff_t b_low = b & hmask;
        /*
         * a * b
         * = ((a_high << hbits) + a_low) * ((b_high << hbits) + b_low)
         * = ((a_high * b_high) << (2 * hbits))
         *   + ((a_high * b_low) << hbits)
         *   + ((a_low * b_high) << hbits)
         *   + (a_low * b_low)
         */
        coeff_t high = a_high * b_high;
        coeff_t hl = a_high * b_low;
        coeff_t lh = a_low * b_high;
        coeff_t low = a_low * b_low;
        coeff_t carry;
        low = coeff_addc(low, ((hl & hmask) << hbits), carry_in, &carry);
        high = coeff_addc(high, (hl >> hbits) & hmask, carry, &carry);
        assert(carry == 0);
        low = coeff_addc(low, ((lh & hmask) << hbits), 0, &carry);
        high = coeff_addc(high, (lh >> hbits) & hmask, carry, &carry);
        assert(carry == 0);
        *highp = high;
        return low;
#endif
}

static coeff_t
coeff_div(coeff_t dividend_high, coeff_t dividend_low, coeff_t divisor)
{
        assert(dividend_high < divisor);
#if UINTMAX_MAX / COEFF_MAX >= COEFF_MAX
        ctassert(UINTMAX_MAX / COEFF_MAX >= COEFF_MAX);
        uintmax_t q =
                ((uintmax_t)dividend_high * BASE + dividend_low) / divisor;
        assert(q <= COEFF_MAX);
        return q;
#else
        /* revisit: use divq on x86 */
        coeff_t h = dividend_high;
        coeff_t l = dividend_low;
        coeff_t q = 0;
        unsigned int shift = COEFF_BITS;
        while (h != 0 || l != 0) {
                while (shift > 0 && h < (coeff_t)1 << (COEFF_BITS - 1)) {
                        /* revisit: optimize with ffs? */
                        shift--;
                        assert((h << 1) >> 1 == h);
                        h <<= 1;
                        h |= l >> (COEFF_BITS - 1);
                        l <<= 1;
                }
                if (h >= divisor) {
                        assert(shift < COEFF_BITS);
                        coeff_t n = h / divisor;
                        h -= n * divisor;
                        assert((n << shift) >> shift == n);
                        q += n << shift;
                        // printf("%zx %zu << %zu\n", (uintmax_t)q,
                        // (uintmax_t)n, (uintmax_t)shift);
                } else if (shift == 0) {
                        break;
                } else {
                        shift--;
                        h <<= 1; /* overflow */
                        h |= l >> (COEFF_BITS - 1);
                        l <<= 1;
                        assert(h < divisor);
                        h -= divisor; /* underflow */
                        q += (coeff_t)1 << shift;
                        // printf("%zx %zu << %zu\n", (uintmax_t)q,
                        // (uintmax_t)1, (uintmax_t)shift);
                }
        }
        assert(shift > 0 || h < divisor); /* note: h is reminder here */
        assert(l == 0);
#if !defined(NDEBUG)
        coeff_t tmp_high;
        coeff_t tmp_low = coeff_mul(&tmp_high, q, divisor, h);
        assert(tmp_high == dividend_high);
        assert(tmp_low == dividend_low);
#endif
        return q;
#endif
}

#define HANDLE_ERROR(call)                                                    \
        do {                                                                  \
                ret = call;                                                   \
                if (ret != 0) {                                               \
                        goto fail;                                            \
                }                                                             \
        } while (0)

#define NO_ERROR(call)                                                        \
        do {                                                                  \
                int ret2 = call;                                              \
                assert(ret2 == 0);                                            \
        } while (0)

#define BIGINT_INITIALIZER                                                    \
        {                                                                     \
                .n = 0                                                        \
        }

#define COPY_IF(cond, a, a0)                                                  \
        do {                                                                  \
                if (cond) {                                                   \
                        BIGINT_SET(&a0, a);                                   \
                        a = &a0;                                              \
                }                                                             \
        } while (false)
#define BIGINT_DEFINE(a) struct bigint a = BIGINT_INITIALIZER
#define BIGINT_ALLOC(a, b) HANDLE_ERROR(bigint_alloc(a, b))
#define BIGINT_SET_UINT(a, b) HANDLE_ERROR(bigint_set_uint(a, b))
#define BIGINT_SET_UINT1(a, b) HANDLE_ERROR(bigint_set_uint1(a, b))
#define BIGINT_SET(a, b) HANDLE_ERROR(bigint_set(a, b))
#define BIGINT_ADD(a, b, c) HANDLE_ERROR(bigint_add(a, b, c))
#define BIGINT_SUB(a, b, c) HANDLE_ERROR(bigint_sub(a, b, c))
#define BIGINT_SUB_NOFAIL(a, b, c) NO_ERROR(bigint_sub(a, b, c))
#define BIGINT_MUL(a, b, c) HANDLE_ERROR(bigint_mul(a, b, c))
#define BIGINT_DIVREM(a, b, c, d) HANDLE_ERROR(bigint_divrem(a, b, c, d))
#define BIGINT_MUL_UINT1(a, b, c) HANDLE_ERROR(bigint_mul_uint1(a, b, c))
#define SHIFT_LEFT_WORDS(a, b, c) HANDLE_ERROR(shift_left_words(a, b, c))

static int
bigint_alloc(struct bigint *a, unsigned int max_digits)
{
        if (max_digits <= a->max) {
                return 0;
        }
        void *p = realloc(a->d, sizeof(*a->d) * max_digits);
        if (p == NULL) {
                return ENOMEM;
        }
        a->d = p;
        a->max = max_digits;
        return 0;
}

void
bigint_init(struct bigint *a)
{
        a->n = 0;
        a->max = 0;
        a->d = NULL;
}

void
bigint_clear(struct bigint *a)
{
        free(a->d);
}

static bool
is_normal(const struct bigint *a)
{
        if (a->n == 0) {
                return true;
        }
        if (a->d[a->n - 1] == 0) {
                // printf("d[%u] = %zd\n", a->n - 1, (intmax_t)a->d[a->n - 1]);
                return false;
        }
        unsigned int i;
        for (i = 0; i < a->n; i++) {
                if (a->d[i] < 0) {
                        // printf("d[%u] = %zd\n", i, (intmax_t)a->d[i]);
                        return false;
                }
                if (a->d[i] > COEFF_MAX) {
                        // printf("d[%u] = %zd\n", i, (intmax_t)a->d[i]);
                        return false;
                }
        }
        return true;
}

int
bigint_cmp(const struct bigint *a, const struct bigint *b)
{
        if (a->n > b->n) {
                return 1;
        }
        if (a->n < b->n) {
                return -1;
        }
        unsigned int n = a->n;
        unsigned int i;
        for (i = 0; i < n; i++) {
                coeff_t av = a->d[n - 1 - i];
                coeff_t bv = b->d[n - 1 - i];
                if (av > bv) {
                        return 1;
                }
                if (av < bv) {
                        return -1;
                }
        }
        return 0;
}

int
bigint_add(struct bigint *c, const struct bigint *a, const struct bigint *b)
{
        unsigned int n = (a->n > b->n) ? a->n : b->n;
        unsigned int i;
        int ret;

        BIGINT_ALLOC(c, n + 1);
        coeff_t carry = 0;
        for (i = 0; i < n; i++) {
                c->d[i] = coeff_addc(dig(a, i), dig(b, i), carry, &carry);
        }
        c->n = n;
        if (carry) {
                assert(c->n < c->max);
                c->d[c->n++] = carry;
        }
        return 0;
fail:
        return ret;
}

int
bigint_sub(struct bigint *c, const struct bigint *a, const struct bigint *b)
{
        unsigned int n = a->n;
        unsigned int i;
        int ret;

        assert(bigint_cmp(a, b) >= 0);
        BIGINT_ALLOC(c, n);
        coeff_t carry = 0;
        for (i = 0; i < n; i++) {
                c->d[i] = coeff_subc(dig(a, i), dig(b, i), carry, &carry);
        }
        c->n = n;
        assert(carry == 0);

        /* normalize */
        while (c->n > 0 && c->d[c->n - 1] == 0) {
                c->n--;
        }
        return 0;
fail:
        return ret;
}

static void
mul1(struct bigint *c, const struct bigint *a, coeff_t n)
{
        assert(is_normal(a));
        if (n == 0) {
                c->n = 0;
                return;
        }
        assert(a->n < c->max || a->n == 0 ||
               a->d[a->n - 1] * n <= COEFF_TYPE_MAX);
        unsigned int i;
        coeff_t carry = 0;
        for (i = 0; i < a->n; i++) {
                c->d[i] = coeff_mul(&carry, a->d[i], n, carry);
        }
        c->n = a->n;
        assert(c->n <= c->max);
        if (carry > 0) {
                assert(c->n < c->max);
                c->d[c->n++] = carry;
        }
        assert(is_normal(c));
}

int
bigint_mul(struct bigint *c, const struct bigint *a, const struct bigint *b)
{
        assert(is_normal(a));
        assert(is_normal(b));
        if (a->n == 0 || b->n == 0) {
                c->n = 0;
                return 0;
        }
        BIGINT_DEFINE(t);
        BIGINT_DEFINE(a0);
        BIGINT_DEFINE(b0);
        int ret;
        COPY_IF(c == a, a, a0);
        COPY_IF(c == b, b, b0);
        assert(c != a);
        assert(c != b);
        BIGINT_ALLOC(c, a->n + b->n + 1);
        BIGINT_ALLOC(&t, a->n + 1);
        mul1(c, a, b->d[0]);
        assert(is_normal(c));
        unsigned int i;
        for (i = 1; i < b->n; i++) {
                if (b->d[i] == 0) {
                        continue;
                }
                mul1(&t, a, b->d[i]);
                assert(is_normal(&t));
                /* c += (t << (base * i)) */
                assert(c->n <= i + t.n);
                assert(i + t.n <= c->max);
                coeff_t carry = 0;
                unsigned int j;
                for (j = 0; j < t.n; j++) {
                        c->d[i + j] = coeff_addc(dig(c, i + j), t.d[j], carry,
                                                 &carry);
                }
                assert(c->n <= i + t.n);
                c->n = i + t.n;
                if (carry) {
                        assert(c->n < c->max);
                        c->d[c->n++] = carry;
                }
                assert(is_normal(c));
        }
        ret = 0;
fail:
        bigint_clear(&t);
        bigint_clear(&a0);
        bigint_clear(&b0);
        return ret;
}

static int
div_normalize(struct bigint *a, struct bigint *b, unsigned int *kp)
{
        assert(b->n != 0);

        unsigned int k = 0;
        int ret;
#if defined(BASE)
#define HALF_BASE (BASE / 2)
#else
#define HALF_BASE ((coeff_t)1 << (BASE_BITS - 1))
#endif
        while (b->d[b->n - 1] < HALF_BASE) {
                k++;
                mul1(b, b, 2);
        }
        if (k > 0) {
                if (a->n > 0) {
                        BIGINT_ALLOC(a, a->n + 1);
                }
                mul1(a, a, (coeff_t)1 << k);
        }
        *kp = k;
        return 0;
fail:
        return ret;
}

static int
shift_left_words(struct bigint *d, const struct bigint *s, unsigned int n)
{
        assert(d != s);
        assert(is_normal(s));
        if (s->n == 0) {
                d->n = 0;
                return 0;
        }
        int ret;
        BIGINT_ALLOC(d, s->n + n);
        unsigned int i;
        for (i = 0; i < n; i++) {
                d->d[i] = 0;
        }
        for (; i < s->n + n; i++) {
                d->d[i] = s->d[i - n];
        }
        d->n = s->n + n;
        assert(is_normal(d));
        return 0;
fail:
        return ret;
}

int
bigint_divrem(struct bigint *q, struct bigint *r, const struct bigint *a,
              const struct bigint *b0)
{
        BIGINT_DEFINE(b);
        BIGINT_DEFINE(b1);
        BIGINT_DEFINE(tmp);
        BIGINT_DEFINE(tmp2);
        unsigned int k;
        int ret;

        COPY_IF(q == b0 || r != b0, b0, b1);
        assert(is_normal(a));
        assert(is_normal(b0));
        assert(b0->n != 0); /* XXX report division-by-zero? */
        BIGINT_SET(r, a);
        if (bigint_cmp(a, b0) < 0) {
                q->n = 0;
                ret = 0;
                goto fail;
        }
        BIGINT_SET(&b, b0);
        ret = div_normalize(r, &b, &k);
        if (ret != 0) {
                goto fail;
        }
        unsigned int n = b.n;
        assert(r->n >= n);
        unsigned int m = r->n - n;
#if !defined(NDEBUG)
        /* assert(r < 2 * (BASE ** m) * b) */
        SHIFT_LEFT_WORDS(&tmp, &b, m);
        BIGINT_ADD(&tmp, &tmp, &tmp);
        assert(bigint_cmp(r, &tmp) < 0);
#endif
        BIGINT_ALLOC(q, m + 1);
        /* tmp = (BASE ** m) * b */
        SHIFT_LEFT_WORDS(&tmp, &b, m);
        int cmp = bigint_cmp(r, &tmp);
        if (cmp >= 0) {
                q->d[m] = 1;
                q->n = m + 1;
                BIGINT_SUB(r, r, &tmp);
        } else {
                q->n = m;
        }
        if (m > 0) {
                unsigned int j = m - 1;
                do {
#if !defined(NDEBUG) && 0
                        /* assert(r < (BASE ** (j + 1)) * b) */
                        SHIFT_LEFT_WORDS(&tmp, &b, j + 1);
                        assert(bigint_cmp(r, &b) < 0);
#endif
                        coeff_t q_j;
                        coeff_t high = r->d[n + j];
                        coeff_t divisor = b.d[n - 1];
                        assert(high <= divisor);
                        if (high >= divisor) {
                                q_j = COEFF_MAX;
                        } else {
                                coeff_t low = r->d[n + j - 1];
                                q_j = coeff_div(high, low, divisor);
                                assert(q_j <= COEFF_MAX);
                        }
                        /* tmp = (BASE ** j) * b */
                        SHIFT_LEFT_WORDS(&tmp, &b, j);
                        /* tmp2 = q_j * tmp */
                        BIGINT_MUL_UINT1(&tmp2, &tmp, q_j);
                        while (bigint_cmp(r, &tmp2) < 0) {
                                q_j--;
                                BIGINT_SUB_NOFAIL(&tmp2, &tmp2, &tmp);
                        }
                        BIGINT_SUB_NOFAIL(r, r, &tmp2);
                        q->d[j] = q_j;
                } while (j-- > 0);
        }
        ret = 0;
        assert(is_normal(q));
        assert(is_normal(r));
        assert(bigint_cmp(r, &b) < 0);
        if (k > 0 && r->n != 0) {
                /* r = r / (2 ** k) */
                BIGINT_SET_UINT1(&tmp, (coeff_t)1 << k);
                BIGINT_DIVREM(r, &tmp2, r, &tmp);
                assert(tmp2.n == 0); /* should be an exact division */
        }
        assert(is_normal(q));
        assert(is_normal(r));
fail:
        bigint_clear(&b);
        bigint_clear(&b1);
        bigint_clear(&tmp);
        bigint_clear(&tmp2);
        return ret;
}

int
bigint_set(struct bigint *d, const struct bigint *s)
{
        int ret;
        BIGINT_ALLOC(d, s->n);
        unsigned int i;
        for (i = 0; i < s->n; i++) {
                d->d[i] = s->d[i];
        }
        d->n = s->n;
fail:
        return ret;
}

int
bigint_set_uint1(struct bigint *a, coeff_t v)
{
        assert(v <= COEFF_MAX);
        if (v == 0) {
                a->n = 0;
                return 0;
        }
        int ret;
        BIGINT_ALLOC(a, 1);
        a->d[0] = v;
        a->n = 1;
fail:
        return ret;
}

int
bigint_set_uint(struct bigint *a, unsigned int v)
{
#if defined(BASE)
        if (v <= COEFF_MAX) {
                return bigint_set_uint1(a, v);
        }
        int ret;
        BIGINT_DEFINE(t);
        BIGINT_DEFINE(bb);
        bigint_set_zero(a);
        BIGINT_SET_UINT1(&bb, 1); /* bb = 1 */
        while (true) {
                unsigned int d = v % BASE;
                BIGINT_SET_UINT1(&t, d);
                BIGINT_MUL(&t, &t, &bb);
                BIGINT_ADD(a, a, &t); /* a += d * bb */
                v /= BASE;
                if (v == 0) {
                        break;
                }
                BIGINT_MUL(&bb, &bb, &base); /* bb *= BASE */
        }
fail:
        bigint_clear(&t);
        bigint_clear(&bb);
        return ret;
#else
        return bigint_set_uint1(a, v);
#endif
}

int
bigint_mul_uint1(struct bigint *d, const struct bigint *a, coeff_t b)
{
        assert(b <= COEFF_MAX);
        int ret;
        BIGINT_ALLOC(d, a->n + 1);
        mul1(d, a, b);
fail:
        return ret;
}

int
bigint_is_zero(const struct bigint *a)
{
        assert(is_normal(a));
        return a->n == 0;
}

void
bigint_set_zero(struct bigint *a)
{
        a->n = 0;
        assert(bigint_is_zero(a));
}

int
bigint_from_str(struct bigint *a, const char *p)
{
#if BASE == 10
        size_t n = strlen(p);
        int ret;
        BIGINT_ALLOC(a, n);
        unsigned int i;
        for (i = 0; i < n; i++) {
                a->d[i] = p[n - i - 1] - '0';
        }
        a->n = n;
        if (a->d[a->n - 1] == 0) {
                a->n--;
        }
        assert(is_normal(a));
fail:
        return ret;
#else
        size_t n = strlen(p);
        int ret;

        BIGINT_DEFINE(tmp);
        a->n = 0; /* a = 0 */
        unsigned int i;
        for (i = 0; i < n; i++) {
                BIGINT_MUL(&tmp, a, &ten);       /* tmp = a * 10 */
                BIGINT_SET_UINT1(a, p[i] - '0'); /* a = digit */
                BIGINT_ADD(a, a, &tmp);          /* a = a + tmp */
        }
        ret = 0;
        assert(is_normal(a));
fail:
        bigint_clear(&tmp);
        return ret;
#endif
}

static size_t
estimate_ndigits(const struct bigint *a)
{
        assert(is_normal(a));
        if (a->n == 0) {
                return 1;
        }
#if BASE == 10
        return a->n;
#else
        /* l(10) = 2.30258509299404568401 */
        return a->n * LOG_BASE / 2.30258509299404568401 + 1;
#endif
}

char *
bigint_to_str(const struct bigint *a)
{
#if BASE == 10
        assert(is_normal(a));
        if (a->n == 0) {
                char *p = malloc(2);
                p[0] = '0';
                p[1] = 0;
                return p;
        }
        char *p = malloc(a->n + 1);
        unsigned int i;
        for (i = 0; i < a->n; i++) {
                p[i] = a->d[a->n - i - 1] + '0';
        }
        p[i] = 0;
        return p;
#else
        assert(is_normal(a));
        size_t sz = estimate_ndigits(a) + 1;
        char *p = malloc(sz);
        if (p == NULL) {
                return NULL;
        }
        if (a->n == 0) {
                p[0] = '0';
                p[1] = 0;
                return p;
        }
        BIGINT_DEFINE(q);
        BIGINT_DEFINE(r);
        int ret;

        unsigned int n = sz;
        BIGINT_SET(&q, a);
        p[--n] = 0;
        do {
                assert(n > 0);
                BIGINT_DIVREM(&q, &r, &q, &ten);
                assert(r.n <= 1);
                char ch = '0' + dig(&r, 0);
                p[--n] = ch;
        } while (q.n != 0);
        if (n > 0) {
                memmove(p, &p[n], sz - n);
        }
        goto done;
fail:
        free(p);
        p = NULL;
done:
        bigint_clear(&q);
        bigint_clear(&r);
        return p;
#endif
}

void
bigint_str_free(char *p)
{
        free(p);
}

/* tests */

static void
print_bigint(const char *heading, const struct bigint *a)
{
        assert(is_normal(a));
        char *p = bigint_to_str(a);
        printf("%s%s\n", heading, p);
        bigint_str_free(p);
}

int
gcd(struct bigint *c, const struct bigint *a0, const struct bigint *b0)
{
        const struct bigint *a = a0;
        const struct bigint *b = b0;
        BIGINT_DEFINE(q);
        struct bigint t[3];
        unsigned int i;
        int ret;

        for (i = 0; i < 3; i++) {
                bigint_init(&t[i]);
        }

        i = 0;
        while (1) {
                // print_bigint("a  =", a);
                // print_bigint("b  =", b);
                BIGINT_DIVREM(&q, &t[i], a, b);
                // print_bigint("a/b=", &q);
                // print_bigint("a%b=", &t[i]);
                if (t[i].n == 0) {
                        break;
                }
                a = b;
                b = &t[i];
                i = (i + 1) % 3;
        };
        BIGINT_SET(c, b);
        assert(is_normal(c));
        ret = 0;
fail:
        bigint_clear(&q);
        for (i = 0; i < 3; i++) {
                bigint_clear(&t[i]);
        }
        return ret;
}

static void
test_str_roundtrip(const char *str)
{
        BIGINT_DEFINE(a);
        int ret = bigint_from_str(&a, str);
        assert(ret == 0);
        char *p = bigint_to_str(&a);
        printf("expected %s\n", str);
        printf("actual   %s\n", p);
        assert(!strcmp(p, str));
        bigint_str_free(p);
        bigint_clear(&a);
}

int
factorial(struct bigint *a, const struct bigint *n)
{
        BIGINT_DEFINE(c);
        BIGINT_DEFINE(t);
        int ret;
        BIGINT_SET_UINT1(a, 1);
        BIGINT_SET(&c, n);
        while (!bigint_is_zero(&c)) {
                BIGINT_MUL(&t, a, &c);
                BIGINT_SET(a, &t);
                BIGINT_SUB_NOFAIL(&c, &c, &one);
        }
fail:
        bigint_clear(&c);
        bigint_clear(&t);
        return ret;
}

#include <time.h>

uint64_t
timestamp(void)
{
        struct timespec tv;
        clock_gettime(CLOCK_REALTIME, &tv);
        return (uint64_t)tv.tv_sec * 1000000000 + tv.tv_nsec;
}

int
bench(void)
{
        unsigned int num = 10000;
        printf("calculating %u!...\n", num);
        BIGINT_DEFINE(a);
        BIGINT_DEFINE(n);
        int ret;
        BIGINT_SET_UINT(&n, num);
        uint64_t start_time = timestamp();
        ret = factorial(&a, &n);
        if (ret != 0) {
                goto fail;
        }
        uint64_t end_time = timestamp();
        printf("took %.03f sec\n",
               (double)(end_time - start_time) / 1000000000);
#if 0
        char *ap = bigint_to_str(&a);
        if (ap == NULL) {
                goto fail;
        }
        char *np = bigint_to_str(&n);
        if (np == NULL) {
                bigint_str_free(ap);
                goto fail;
        }
        printf("%s! = %s\n", np, ap);
        bigint_str_free(ap);
        bigint_str_free(np);
#endif
fail:
        bigint_clear(&a);
        bigint_clear(&n);
        printf("ret %d\n", ret);
        return ret;
}

void
assert_eq(const struct bigint *a, const char *str)
{
        char *p = bigint_to_str(a);
        assert(p != NULL);
        if (strcmp(p, str)) {
                printf("unexpected value\n");
                printf("    actual  : %s\n", p);
                printf("    expected: %s\n", str);
                abort();
        }
        bigint_str_free(p);
}

int
main(void)
{
        const char *a_str =
                "12409715069012348970189741096590126450986902431123456";
        const char *b_str =
                "21434109785019758904721590874321400983729087987654";
        struct bigint a;
        struct bigint b;
        struct bigint s;
        struct bigint d;
        struct bigint prod;
        struct bigint q;
        struct bigint r;
        struct bigint tmp;
        struct bigint tmp2;
        int ret;

#if BASE_BITS == 64
        coeff_t high;
        coeff_t low = coeff_mul(&high, 0xffffffffffffffff, 0xffffffffffffffff,
                                0xffffffffffffffff);
        assert(high == 0xffffffffffffffff);
        assert(low == 0);
        coeff_t t = coeff_div(0xFFFFFFFFFFFFFFFE, 0x1, 0xFFFFFFFFFFFFFFFF);
        assert(t == 0xFFFFFFFFFFFFFFFF);
        t = coeff_div(0, 0xFFFFFFFFFFFFFFFE, 0xFFFFFFFFFFFFFFFF);
        assert(t == 0);
        t = coeff_div(100, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
        assert(t == 101);
        t = coeff_div(0xFFFFFFFF00000000, 0, 0xFFFFFFFFFFFFFFFF);
        assert(t == 0xFFFFFFFF00000000);
        t = coeff_div(99, 0xFFFFFFFFFFFFFFFF, 100);
        assert(t == 0xFFFFFFFFFFFFFFFF);
        t = coeff_div(0x119A529501BEF42E, 0x55F88ADF038312C9,
                      0x43210FE1424432DD);
        assert(t == 0x43210FE1424432DD);
        t = coeff_div(0x119A52, 0x55F88ADF038312C9, 0x43210FE14);
        assert(t == 0x43210EF0E0969);
        t = coeff_div(0, 9123456789012346, 10000);
        assert(t == 912345678901);
        t = coeff_div(0, 9999, 10000);
        assert(t == 0);
        t = coeff_div(0, 0, 10000);
        assert(t == 0);
#endif
#if BASE == 10
        {
                BIGINT_DEFINE(q);
                BIGINT_DEFINE(r);
                struct bigint a = {
                        .n = 3,
                        .d =
                                (coeff_t[]){
                                        8,
                                        0,
                                        9,
                                },
                };
                struct bigint b = {
                        .n = 2,
                        .d =
                                (coeff_t[]){
                                        2,
                                        9,
                                },
                };
                print_bigint("dividend ", &a);
                print_bigint("divisor  ", &b);
                ret = bigint_divrem(&q, &r, &a, &b);
                assert(ret == 0);
                assert_eq(&q, "9");
                assert_eq(&r, "80");
                bigint_clear(&q);
                bigint_clear(&r);
        }
#endif

        test_str_roundtrip("0");
        test_str_roundtrip("100000000000000000000000");
        test_str_roundtrip(a_str);
        test_str_roundtrip(b_str);

        bigint_init(&a);
        bigint_init(&b);
        bigint_init(&s);
        bigint_init(&d);
        bigint_init(&prod);
        bigint_init(&q);
        bigint_init(&r);
        bigint_init(&tmp);
        bigint_init(&tmp2);
        assert(bigint_cmp(&a, &a) == 0);
        assert(bigint_cmp(&b, &b) == 0);
        assert(bigint_cmp(&a, &b) == 0);
        assert(bigint_cmp(&b, &a) == 0);
        bigint_from_str(&a, a_str);
        assert(bigint_cmp(&a, &b) > 0);
        assert(bigint_cmp(&b, &a) < 0);
        bigint_from_str(&b, b_str);
        assert(bigint_cmp(&a, &a) == 0);
        assert(bigint_cmp(&b, &b) == 0);
        assert(bigint_cmp(&a, &b) > 0);
        assert(bigint_cmp(&b, &a) < 0);

        /* add and sub */
        bigint_add(&s, &a, &b);
        assert(bigint_cmp(&s, &a) > 0);
        assert(bigint_cmp(&s, &b) > 0);
        bigint_sub(&d, &s, &a);
        assert(bigint_cmp(&d, &b) == 0);
        bigint_sub(&d, &s, &zero);
        assert(bigint_cmp(&d, &s) == 0);
        bigint_sub(&d, &s, &s);
        assert(bigint_cmp(&d, &zero) == 0);

        /* mul */
        ret = bigint_mul(&prod, &a, &b);
        assert(ret == 0);
        ret = bigint_divrem(&q, &r, &prod, &a);
        assert(ret == 0);
        assert(bigint_cmp(&q, &b) == 0);
        assert(bigint_cmp(&r, &zero) == 0);
        ret = bigint_divrem(&q, &r, &prod, &b);
        assert(ret == 0);
        assert(bigint_cmp(&q, &a) == 0);
        assert(bigint_cmp(&r, &zero) == 0);

        /* divrem */
        ret = bigint_divrem(&q, &r, &a, &one);
        assert(ret == 0);
        assert(bigint_cmp(&q, &a) == 0);
        assert(bigint_cmp(&r, &zero) == 0);
        ret = bigint_divrem(&q, &r, &a, &b);
        assert(ret == 0);
        ret = bigint_mul(&tmp, &q, &b);
        assert(ret == 0);
        ret = bigint_add(&tmp, &tmp, &r);
        assert(ret == 0);
        assert(bigint_cmp(&tmp, &a) == 0);

        ret = bigint_divrem(&q, &r, &b, &a);
        assert(ret == 0);
        assert(bigint_cmp(&q, &zero) == 0);
        assert(bigint_cmp(&r, &b) == 0);

        ret = bigint_divrem(&q, &r, &a, &a);
        assert(ret == 0);
        assert(bigint_cmp(&q, &one) == 0);
        assert(bigint_cmp(&r, &zero) == 0);

        ret = bigint_divrem(&q, &r, &b, &b);
        assert(ret == 0);
        assert(bigint_cmp(&q, &one) == 0);
        assert(bigint_cmp(&r, &zero) == 0);

        ret = gcd(&tmp, &a, &b);
        assert(ret == 0);
        print_bigint("a        = ", &a);
        print_bigint("b        = ", &b);
        print_bigint("gcd(a,b) = ", &tmp);

        ret = bigint_from_str(
                &a, "533509908571101979294464811598952141168153495025132870832"
                    "519126598141168533509908571101979294464811598952141168");
        assert(ret == 0);
        ret = bigint_from_str(
                &b, "533509908571101979294464811598952141168533509908571101979"
                    "294464811598952141168533509908571101979294464811598952141"
                    "168533509908571101979294464811598952141168533509908571101"
                    "979294464811598952141168000");
        assert(ret == 0);
        ret = gcd(&tmp, &a, &b);
        assert(ret == 0);
        print_bigint("a        = ", &a);
        print_bigint("b        = ", &b);
        print_bigint("gcd(a,b) = ", &tmp);
        ret = bigint_from_str(&tmp2, "1975308624");
        assert(ret == 0);
        assert(bigint_cmp(&tmp, &tmp2) == 0);

        bigint_clear(&a);
        bigint_clear(&b);
        bigint_clear(&s);
        bigint_clear(&d);
        bigint_clear(&prod);
        bigint_clear(&q);
        bigint_clear(&r);
        bigint_clear(&tmp);
        bigint_clear(&tmp2);

        bench();
}
