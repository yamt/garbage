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

#define BIGINT_INITIALIZER0                                                   \
        {                                                                     \
                .n = 0                                                        \
        }

#define BIGINT_INITIALIZER(N, ...)                                            \
        {                                                                     \
                .n = N, .d = (coeff_t[]){__VA_ARGS__},                        \
        }

const struct bigint g_zero = BIGINT_INITIALIZER0;
const struct bigint g_one = BIGINT_INITIALIZER(1, 1);
const struct bigint g_base = BIGINT_INITIALIZER(2, 0, 1);
#if COEFF_MAX == 9
const struct bigint g_ten = BIGINT_INITIALIZER(2, 0, 1);
#endif
#if COEFF_MAX >= 10
const struct bigint g_ten = BIGINT_INITIALIZER(1, 10);
#endif
#if COEFF_MAX >= 16
const struct bigint g_16 = BIGINT_INITIALIZER(1, 16);
#elif defined(BASE) && 16 / BASE < BASE
const struct bigint g_16 = BIGINT_INITIALIZER(2, 16 % BASE, 16 / BASE);
#endif

static coeff_t
dig(const struct bigint *a, size_t i)
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

#define COPY_IF(cond, a, a0)                                                  \
        do {                                                                  \
                if (cond) {                                                   \
                        BIGINT_SET(&a0, a);                                   \
                        a = &a0;                                              \
                }                                                             \
        } while (false)
#define BIGINT_DEFINE(a) struct bigint a = BIGINT_INITIALIZER0
#define BIGINT_ALLOC(a, b) HANDLE_ERROR(bigint_alloc(a, b))
#define BIGINT_SET_UINT(a, b) HANDLE_ERROR(bigint_set_uint(a, b))
#define BIGINT_FROM_STR(a, b) HANDLE_ERROR(bigint_from_str(a, b))
#define BIGINT_TO_UINT(a, b) HANDLE_ERROR(bigint_to_uint(a, b))
#define BIGINT_SET_UINT1(a, b) HANDLE_ERROR(bigint_set_uint1(a, b))
#define BIGINT_SET(a, b) HANDLE_ERROR(bigint_set(a, b))
#define BIGINT_ADD(a, b, c) HANDLE_ERROR(bigint_add(a, b, c))
#define BIGINT_SUB(a, b, c) HANDLE_ERROR(bigint_sub(a, b, c))
#define BIGINT_SUB_NOFAIL(a, b, c) NO_ERROR(bigint_sub(a, b, c))
#define BIGINT_MUL(a, b, c) HANDLE_ERROR(bigint_mul(a, b, c))
#define BIGINT_DIVREM(a, b, c, d) HANDLE_ERROR(bigint_divrem(a, b, c, d))
#define BIGINT_ROOTINT(a, b, c) HANDLE_ERROR(bigint_rootint(a, b, c))
#define BIGINT_POWINT(a, b, c) HANDLE_ERROR(bigint_powint(a, b, c))
#define BIGINT_MUL_UINT1(a, b, c) HANDLE_ERROR(bigint_mul_uint1(a, b, c))
#define SHIFT_LEFT_WORDS(a, b, c) HANDLE_ERROR(shift_left_words(a, b, c))

__attribute__((unused)) static void
bigint_poison(struct bigint *a)
{
#if !defined(NDEBUG)
        memset(a->d + a->n, 0xaa, (a->max - a->n) * sizeof(*a->d));
#endif
}

static int
bigint_alloc(struct bigint *a, size_t max_digits)
{
        if (max_digits <= a->max) {
                return 0;
        }
        if (max_digits > SIZE_MAX / sizeof(*a->d)) {
                return EOVERFLOW;
        }
        void *p = realloc(a->d, sizeof(*a->d) * max_digits);
        if (p == NULL) {
                return ENOMEM;
        }
        a->d = p;
        a->max = max_digits;
        bigint_poison(a);
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
        size_t i;
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
        size_t n = a->n;
        size_t i;
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
        size_t n = (a->n > b->n) ? a->n : b->n;
        size_t i;
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
        size_t n = a->n;
        size_t i;
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
        size_t i;
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
        if (a->n > SIZE_MAX - 1 || b->n > SIZE_MAX - 1 - a->n) {
                ret = EOVERFLOW;
                goto fail;
        }
        BIGINT_ALLOC(c, a->n + b->n + 1);
        BIGINT_ALLOC(&t, a->n + 1);
        /*
         * initialize with 0, as b->d might contain 0s, for which
         * the following logic can leave the corresponding c->d elements
         * uninitialized.
         */
        memset(c->d, 0, c->max * sizeof(*c->d));
        mul1(c, a, b->d[0]);
        assert(is_normal(c));
        size_t i;
        for (i = 1; i < b->n; i++) {
                assert(i < b->n);
                if (b->d[i] == 0) {
                        continue;
                }
                mul1(&t, a, b->d[i]);
                assert(is_normal(&t));
                /* c += t * (base ^ i) */
                assert(c->n <= i + t.n);
                assert(i + t.n <= c->max);
                coeff_t carry = 0;
                size_t j;
                for (j = 0; j < t.n; j++) {
                        assert(i + j < c->max);
                        assert(j < t.n);
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
                        if (a->n > SIZE_MAX - 1) {
                                ret = EOVERFLOW;
                                goto fail;
                        }
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
shift_left_words(struct bigint *d, const struct bigint *s, size_t n)
{
        assert(d != s);
        assert(is_normal(s));
        if (s->n == 0) {
                d->n = 0;
                return 0;
        }
        int ret;
        if (n > SIZE_MAX - s->n) {
                return EOVERFLOW;
        }
        BIGINT_ALLOC(d, s->n + n);
        size_t i;
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

        COPY_IF(q == b0 || r == b0, b0, b1);
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
        size_t n = b.n;
        assert(r->n >= n);
        size_t m = r->n - n;
#if !defined(NDEBUG)
        /* assert(r < 2 * (BASE ** m) * b) */
        SHIFT_LEFT_WORDS(&tmp, &b, m);
        BIGINT_ADD(&tmp, &tmp, &tmp);
        assert(bigint_cmp(r, &tmp) < 0);
#endif
        assert(n > 0);
        assert(m < SIZE_MAX);
        BIGINT_ALLOC(q, m + 1);
        SHIFT_LEFT_WORDS(&tmp, &b, m); /* tmp = (BASE ** m) * b */
        if (bigint_cmp(r, &tmp) >= 0) {
                q->d[m] = 1;
                q->n = m + 1;
                BIGINT_SUB(r, r, &tmp);
        } else {
                q->n = m;
        }
        for (; m > 0; m--) {
                size_t j = m - 1;
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
                SHIFT_LEFT_WORDS(&tmp, &b, j);      /* tmp = (BASE ** j) * b */
                BIGINT_MUL_UINT1(&tmp2, &tmp, q_j); /* tmp2 = q_j * tmp */
                while (bigint_cmp(r, &tmp2) < 0) {
                        q_j--;
                        BIGINT_SUB_NOFAIL(&tmp2, &tmp2, &tmp);
                }
                BIGINT_SUB_NOFAIL(r, r, &tmp2);
                q->d[j] = q_j;
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
bigint_rootint(struct bigint *s, const struct bigint *m, unsigned int k)
{
        assert(bigint_cmp(m, &g_zero) > 0);
        assert(k >= 2);
        BIGINT_DEFINE(m1);
        BIGINT_DEFINE(bk);
        BIGINT_DEFINE(bk_minus_1);
        BIGINT_DEFINE(u);
        BIGINT_DEFINE(tmp);
        BIGINT_DEFINE(unused);
        int ret;
        COPY_IF(s == m, m, m1);
        BIGINT_SET_UINT(&bk, k);
        BIGINT_SET_UINT(&bk_minus_1, k - 1);
        BIGINT_SET(&u, m);
        do {
                BIGINT_SET(s, &u);
                BIGINT_MUL(&u, &u, &bk_minus_1);
                /* tmp = m / (s ^ (k - 1)) */
                BIGINT_SET(&tmp, m);
                unsigned int i;
                for (i = 0; i < k - 1; i++) {
                        BIGINT_DIVREM(&tmp, &unused, &tmp, s);
                }
                BIGINT_ADD(&u, &u, &tmp);
                BIGINT_DIVREM(&u, &unused, &u, &bk);
        } while (bigint_cmp(&u, s) < 0);
        ret = 0;
fail:
        bigint_clear(&bk);
        bigint_clear(&bk_minus_1);
        bigint_clear(&u);
        bigint_clear(&tmp);
        bigint_clear(&unused);
        bigint_clear(&m1);
        return ret;
}

int
bigint_powint(struct bigint *s, const struct bigint *m, unsigned int k)
{
        BIGINT_DEFINE(m1);
        unsigned int i;
        int ret;
        COPY_IF(s == m, m, m1);
        BIGINT_SET_UINT(s, 1);
        for (i = 0; i < k; i++) {
                BIGINT_MUL(s, s, m);
        }
        ret = 0;
fail:
        bigint_clear(&m1);
        return ret;
}

int
bigint_set(struct bigint *d, const struct bigint *s)
{
        int ret;
        BIGINT_ALLOC(d, s->n);
        size_t i;
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
bigint_set_uint(struct bigint *a, uintmax_t v)
{
#if defined(BASE)
        if (v <= COEFF_MAX) {
                return bigint_set_uint1(a, v);
        }
        int ret;
        BIGINT_DEFINE(t);
        BIGINT_DEFINE(bb);
        bigint_set_zero(a);       /* a = 0 */
        BIGINT_SET_UINT1(&bb, 1); /* bb = 1 */
        while (true) {
                coeff_t d = v % BASE;
                BIGINT_SET_UINT1(&t, d);
                BIGINT_MUL(&t, &t, &bb);
                BIGINT_ADD(a, a, &t); /* a += d * bb */
                v /= BASE;
                if (v == 0) {
                        break;
                }
                BIGINT_MUL(&bb, &bb, &g_base); /* bb *= BASE */
        }
fail:
        bigint_clear(&t);
        bigint_clear(&bb);
        return ret;
#else
        assert(v <= COEFF_MAX);
        return bigint_set_uint1(a, v);
#endif
}

int
bigint_to_uint1(const struct bigint *a, coeff_t *vp)
{
        if (a->n == 0) {
                *vp = 0;
                return 0;
        }
        if (a->n > 1) {
                return EOVERFLOW;
        }
        if (a->d[0] > COEFF_MAX) {
                return EOVERFLOW;
        }
        *vp = a->d[0];
        return 0;
}

int
bigint_to_uint(const struct bigint *a, uintmax_t *vp)
{
#if COEFF_MAX >= UINTMAX_MAX
        coeff_t c;
        int ret = bigint_to_uint1(a, &c);
        if (ret == 0) {
                *vp = c;
        }
        return ret;
#else
        uintmax_t v = 0;
        size_t i;
        for (i = 0; i < a->n; i++) {
#if defined(BASE)
                if (UINTMAX_MAX / BASE < v) {
                        return EOVERFLOW;
                }
                v *= BASE;
#else
                if ((UINTMAX_MAX >> COEFF_BITS) < v) {
                        return EOVERFLOW;
                }
                v <<= COEFF_BITS;
#endif
                if (UINTMAX_MAX - v < (uintmax_t)a->d[i]) {
                        return EOVERFLOW;
                }
                v += a->d[a->n - i - 1];
        }
        *vp = v;
        return 0;
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

static int
digit_from_chr(char ch, unsigned int *p)
{
        unsigned int x;
        switch (ch) {
        case '0':
                x = 0;
                break;
        case '1':
                x = 1;
                break;
        case '2':
                x = 2;
                break;
        case '3':
                x = 3;
                break;
        case '4':
                x = 4;
                break;
        case '5':
                x = 5;
                break;
        case '6':
                x = 6;
                break;
        case '7':
                x = 7;
                break;
        case '8':
                x = 8;
                break;
        case '9':
                x = 9;
                break;
        case 'a':
                x = 10;
                break;
        case 'b':
                x = 11;
                break;
        case 'c':
                x = 12;
                break;
        case 'd':
                x = 13;
                break;
        case 'e':
                x = 14;
                break;
        case 'f':
                x = 15;
                break;
        default:
                return EINVAL;
        }
        *p = x;
        return 0;
}

static char
digit_chr(unsigned int x)
{
        assert(x < 16);
        return "0123456789abcdef"[x];
}

int
bigint_from_str_base(struct bigint *a, const struct bigint *base,
                     const char *p)
{
        size_t n = strlen(p);
        int ret;

        BIGINT_DEFINE(tmp);
        a->n = 0; /* a = 0 */
        size_t i;
        for (i = 0; i < n; i++) {
                BIGINT_MUL(&tmp, a, base); /* tmp = a * base */
                unsigned int x;
                ret = digit_from_chr(p[i], &x);
                if (ret != 0) {
                        goto fail;
                }
                BIGINT_SET_UINT(a, x); /* a = digit */
                if (bigint_cmp(a, base) >= 0) {
                        ret = EINVAL;
                        goto fail;
                }
                BIGINT_ADD(a, a, &tmp); /* a = a + tmp */
        }
        ret = 0;
        assert(is_normal(a));
fail:
        bigint_clear(&tmp);
        return ret;
}

int
bigint_from_str(struct bigint *a, const char *p)
{
#if BASE == 10
        size_t n = strlen(p);
        int ret;
        BIGINT_ALLOC(a, n);
        size_t i;
        for (i = 0; i < n; i++) {
                unsigned int x;
                ret = digit_from_chr(p[n - i - 1], &x);
                if (ret != 0) {
                        goto fail;
                }
                if (x > COEFF_MAX) {
                        ret = EINVAL;
                        goto fail;
                }
                a->d[i] = x;
        }
        a->n = n;
        if (a->d[a->n - 1] == 0) {
                a->n--;
        }
        assert(is_normal(a));
fail:
        return ret;
#else
        return bigint_from_str_base(a, &g_ten, p);
#endif
}

int
bigint_from_hex_str(struct bigint *a, const char *p)
{
        return bigint_from_str_base(a, &g_16, p);
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
        /* XXX check overflow */
        return a->n * LOG_BASE / 2.30258509299404568401 + 1;
#endif
}

int
bigint_to_str_impl(char *p, size_t sz, const struct bigint *a,
                   const struct bigint *base)
{
        BIGINT_DEFINE(q);
        BIGINT_DEFINE(r);
        int ret;

        size_t n = sz;
        BIGINT_SET(&q, a);
        p[--n] = 0;
        do {
                assert(n > 0);
                BIGINT_DIVREM(&q, &r, &q, base);
                uintmax_t u;
                BIGINT_TO_UINT(&r, &u);
                char ch = digit_chr(u);
                p[--n] = ch;
        } while (q.n != 0);
        if (n > 0) {
                memmove(p, &p[n], sz - n);
        }
        ret = 0;
fail:
        bigint_clear(&q);
        bigint_clear(&r);
        return ret;
}

char *
bigint_to_str(const struct bigint *a)
{
        assert(is_normal(a));
        size_t sz = estimate_ndigits(a) + 1; /* XXX check overflow */
        char *p = malloc(sz);
        if (p == NULL) {
                return NULL;
        }
        if (a->n == 0) {
                p[0] = '0';
                p[1] = 0;
                return p;
        }
#if BASE == 10
        size_t i;
        for (i = 0; i < a->n; i++) {
                p[i] = a->d[a->n - i - 1] + '0';
        }
        p[i] = 0;
        return p;
#else
        if (bigint_to_str_impl(p, sz, a, &g_ten)) {
                free(p);
                return NULL;
        }
        return p;
#endif
}

static size_t
estimate_ndigits_hex(const struct bigint *a)
{
        assert(is_normal(a));
        if (a->n == 0) {
                return 1;
        }
#if BASE == 10
        return a->n;
#else
        /* l(16) = 2.77258872223978123766 */
        /* XXX check overflow */
        return a->n * LOG_BASE / 2.77258872223978123766 + 1;
#endif
}

char *
bigint_to_hex_str(const struct bigint *a)
{
        assert(is_normal(a));
        size_t sz = estimate_ndigits_hex(a) + 1; /* XXX check overflow */
        char *p = malloc(sz);
        if (p == NULL) {
                return NULL;
        }
        if (a->n == 0) {
                p[0] = '0';
                p[1] = 0;
                return p;
        }
        if (bigint_to_str_impl(p, sz, a, &g_16)) {
                free(p);
                return NULL;
        }
        return p;
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
        size_t i;
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

int
fixed_point_sqrt(void)
{
        const char *scale_str = "1000000000000";
        BIGINT_DEFINE(scale);
        BIGINT_DEFINE(t);
        int ret;
        BIGINT_FROM_STR(&scale, scale_str);
        unsigned int i;
        for (i = 1; i < 256; i++) {
                BIGINT_SET_UINT(&t, i);
                BIGINT_MUL(&t, &t, &scale);
                BIGINT_MUL(&t, &t, &scale);
                BIGINT_ROOTINT(&t, &t, 2);
                char *p = bigint_to_str(&t);
                if (p == NULL) {
                        ret = ENOMEM;
                        goto fail;
                }
                printf("sqrt(%3u) * %s = %s\n", i, scale_str, p);
                bigint_str_free(p);
        }
fail:
        bigint_clear(&scale);
        bigint_clear(&t);
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

static void
test_hex_str_roundtrip(const char *str)
{
        BIGINT_DEFINE(a);
        int ret = bigint_from_hex_str(&a, str);
        assert(ret == 0);
        char *p = bigint_to_hex_str(&a);
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
        int ret;
        BIGINT_SET_UINT1(a, 1);
        BIGINT_SET(&c, n);
        while (!bigint_is_zero(&c)) {
                BIGINT_MUL(a, a, &c);
                BIGINT_SUB_NOFAIL(&c, &c, &g_one);
        }
fail:
        bigint_clear(&c);
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
        struct bigint c;
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
                struct bigint a = BIGINT_INITIALIZER(3, 8, 0, 9);
                struct bigint b = BIGINT_INITIALIZER(2, 2, 9);
                print_bigint("dividend ", &a);
                print_bigint("divisor  ", &b);
                ret = bigint_divrem(&q, &r, &a, &b);
                assert(ret == 0);
                assert_eq(&q, "9");
                assert_eq(&r, "80");
                bigint_clear(&q);
                bigint_clear(&r);
        }
        {
                BIGINT_DEFINE(q);
                BIGINT_DEFINE(r);
                struct bigint a = BIGINT_INITIALIZER(3, 0, 1, 8);
                struct bigint b = BIGINT_INITIALIZER(2, 9, 9);
                print_bigint("dividend ", &a);
                print_bigint("divisor  ", &b);
                ret = bigint_divrem(&q, &r, &a, &b);
                assert(ret == 0);
                assert_eq(&q, "8");
                assert_eq(&r, "18");
                bigint_clear(&q);
                bigint_clear(&r);
        }
#endif

        assert(is_normal(&g_zero));
        assert(is_normal(&g_one));
        assert(is_normal(&g_ten));
        assert(is_normal(&g_base));
        assert_eq(&g_zero, "0");
        assert_eq(&g_one, "1");
        assert_eq(&g_ten, "10");

        test_str_roundtrip("0");
        test_str_roundtrip("100000000000000000000000");
        test_str_roundtrip(a_str);
        test_str_roundtrip(b_str);

        test_hex_str_roundtrip("0");
        test_hex_str_roundtrip("100000000000000000000000");
        test_hex_str_roundtrip(a_str);
        test_hex_str_roundtrip(b_str);
        test_hex_str_roundtrip("ab00cd00ef0012003400560077");
        test_hex_str_roundtrip("fffffffffffffffffffffff");
        test_hex_str_roundtrip(
                "fffffffffffffffffffffff00000000000000000000000000000000000000"
                "0000000000000000000000000000000000000000000000000000000000000"
                "00000000000000000000000000000000000000000000000000fffffffffff"
                "fffffffffffffffffffffffffff");

        bigint_init(&a);
        bigint_init(&b);
        bigint_init(&c);
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
        {
                char *p;
                p = bigint_to_hex_str(&a);
                assert(!strcmp(
                        p, "212b125567c1932ed6f400b9e883b365e3a365bac800"));
                p = bigint_to_hex_str(&b);
                assert(!strcmp(p,
                               "eaa72b959fd1535970dea1d15024b7c1325a43fc6"));
        }

        ret = bigint_from_str(&tmp, "1a");
        assert(ret == EINVAL);
        ret = bigint_from_hex_str(&tmp, "1ag");
        assert(ret == EINVAL);

        ret = bigint_set_uint(&tmp, COEFF_MAX);
        assert(ret == 0);
        ret = bigint_add(&tmp, &tmp, &g_one);
        assert(ret == 0);
        assert(!bigint_cmp(&g_base, &tmp));

        ret = bigint_set_uint(&tmp, 0);
        assert(ret == 0);
        assert_eq(&tmp, "0");
        ret = bigint_set_uint(&tmp, 9);
        assert(ret == 0);
        assert_eq(&tmp, "9");
        ret = bigint_set_uint(&tmp, 10);
        assert(ret == 0);
        assert_eq(&tmp, "10");
        ret = bigint_set_uint(&tmp, 11);
        assert(ret == 0);
        assert_eq(&tmp, "11");
        ret = bigint_set_uint(&tmp, 100);
        assert(ret == 0);
        assert_eq(&tmp, "100");

        /* add and sub */
        bigint_add(&s, &a, &b);
        assert(bigint_cmp(&s, &a) > 0);
        assert(bigint_cmp(&s, &b) > 0);
        bigint_sub(&d, &s, &a);
        assert(bigint_cmp(&d, &b) == 0);
        bigint_sub(&d, &s, &g_zero);
        assert(bigint_cmp(&d, &s) == 0);
        bigint_sub(&d, &s, &s);
        assert(bigint_cmp(&d, &g_zero) == 0);

        /* mul */
        ret = bigint_from_str(&c, "111398900663392315947914998");
        assert(ret == 0);
        ret = bigint_mul(&prod, &c, &c);
        assert(ret == 0);
        assert_eq(&prod,
                  "12409715069012348970189741000142675791423583433340004");
        ret = bigint_mul(&prod, &g_one, &c);
        assert(ret == 0);
        print_bigint("c     = ", &c);
        print_bigint("1 * c = ", &prod);
        assert(bigint_cmp(&prod, &c) == 0);
        ret = bigint_mul(&prod, &g_one, &g_one);
        assert(ret == 0);
        assert(bigint_cmp(&prod, &g_one) == 0);
        ret = bigint_mul(&prod, &c, &g_one);
        assert(ret == 0);
        assert(bigint_cmp(&prod, &c) == 0);
        ret = bigint_mul(&prod, &c, &c);
        assert(ret == 0);
        assert_eq(&prod,
                  "12409715069012348970189741000142675791423583433340004");

        ret = bigint_mul(&prod, &a, &b);
        assert(ret == 0);
        ret = bigint_divrem(&q, &r, &prod, &a);
        assert(ret == 0);
        assert(bigint_cmp(&q, &b) == 0);
        assert(bigint_cmp(&r, &g_zero) == 0);
        ret = bigint_divrem(&q, &r, &prod, &b);
        assert(ret == 0);
        assert(bigint_cmp(&q, &a) == 0);
        assert(bigint_cmp(&r, &g_zero) == 0);

        ret = bigint_from_hex_str(&tmp, "100000000000000000000000000000000");
        assert(ret == 0);
        ret = bigint_from_str(&tmp2,
                              "340282366920938463463374607431768211456");
        assert(ret == 0);
        assert(bigint_cmp(&tmp, &tmp2) == 0);
        int i;
        for (i = 1; i < 10; i++) {
                ret = bigint_set_uint(&tmp, i);
                assert(ret == 0);
                bigint_poison(&tmp);
                ret = bigint_mul(&tmp, &tmp, &tmp2);
                assert(ret == 0);
                {
                        char *p = bigint_to_hex_str(&tmp);
                        assert(p != NULL);
                        printf("p %s\n", p);
                        assert(!strcmp(p + 1,
                                       "00000000000000000000000000000000"));
                        bigint_str_free(p);
                }
        }

        /* divrem */
        ret = bigint_divrem(&q, &r, &a, &g_one);
        assert(ret == 0);
        assert(bigint_cmp(&q, &a) == 0);
        assert(bigint_cmp(&r, &g_zero) == 0);
        ret = bigint_divrem(&q, &r, &a, &b);
        assert(ret == 0);
        ret = bigint_mul(&tmp, &q, &b);
        assert(ret == 0);
        ret = bigint_add(&tmp, &tmp, &r);
        assert(ret == 0);
        assert(bigint_cmp(&tmp, &a) == 0);

        ret = bigint_divrem(&q, &r, &b, &a);
        assert(ret == 0);
        assert(bigint_cmp(&q, &g_zero) == 0);
        assert(bigint_cmp(&r, &b) == 0);

        ret = bigint_divrem(&q, &r, &a, &a);
        assert(ret == 0);
        assert(bigint_cmp(&q, &g_one) == 0);
        assert(bigint_cmp(&r, &g_zero) == 0);

        ret = bigint_divrem(&q, &r, &b, &b);
        assert(ret == 0);
        assert(bigint_cmp(&q, &g_one) == 0);
        assert(bigint_cmp(&r, &g_zero) == 0);

        fixed_point_sqrt();

        /* rootint */
        unsigned int k;
        for (k = 2; k < 100; k++) {
                uint64_t start_time = timestamp();
                print_bigint("a                             = ", &a);
                printf("k                             = %u\n", k);
                ret = bigint_rootint(&q, &a, k);
                assert(ret == 0);
                print_bigint("rootint(a, k)                 = ", &q);
                ret = bigint_powint(&tmp, &q, k);
                assert(ret == 0);
                print_bigint("powint(rootint(a, k), k)      = ", &tmp);
                assert(bigint_cmp(&tmp, &a) <= 0);
                ret = bigint_add(&tmp, &q, &g_one);
                assert(ret == 0);
                ret = bigint_powint(&tmp, &tmp, k);
                assert(ret == 0);
                print_bigint("powint(rootint(a, k) + 1), k) = ", &tmp);
                assert(bigint_cmp(&tmp, &a) > 0);
                uint64_t end_time = timestamp();
                printf("took %.03f sec\n",
                       (double)(end_time - start_time) / 1000000000);
        }

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
        bigint_clear(&c);
        bigint_clear(&s);
        bigint_clear(&d);
        bigint_clear(&prod);
        bigint_clear(&q);
        bigint_clear(&r);
        bigint_clear(&tmp);
        bigint_clear(&tmp2);

        bench();
}
