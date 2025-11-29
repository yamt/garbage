/*
 * https://members.loria.fr/PZimmermann/mca/mca-cup-0.5.9.pdf
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mp.h"

#define ctassert(a) _Static_assert(a, #a)

#if defined(BASE)
ctassert(COEFF_MAX == BASE - 1);
#endif

#define MPN_INITIALIZER0                                                   \
        {                                                                     \
                .n = 0                                                        \
        }

#define MPN_INITIALIZER(N, ...)                                            \
        {                                                                     \
                .n = N, .d = (coeff_t[]){__VA_ARGS__},                        \
        }

const struct mpn g_zero = MPN_INITIALIZER0;
const struct mpn g_one = MPN_INITIALIZER(1, 1);
const struct mpn g_base = MPN_INITIALIZER(2, 0, 1);
#if COEFF_MAX == 9
const struct mpn g_ten = MPN_INITIALIZER(2, 0, 1);
#endif
#if COEFF_MAX >= 10
const struct mpn g_ten = MPN_INITIALIZER(1, 10);
#endif
#if COEFF_MAX >= 16
const struct mpn g_16 = MPN_INITIALIZER(1, 16);
#elif defined(BASE) && 16 / BASE < BASE
const struct mpn g_16 = MPN_INITIALIZER(2, 16 % BASE, 16 / BASE);
#endif

static coeff_t
dig(const struct mpn *a, mp_size_t i)
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

static int mpn_set_uint1(struct mpn *a, coeff_t v);
static int mpn_to_uint1(const struct mpn *a, coeff_t *vp);
static int mpn_mul_uint1(struct mpn *d, const struct mpn *a,
                            coeff_t b);

#define MPN_SET_UINT1(a, b) HANDLE_ERROR(mpn_set_uint1(a, b))
#define MPN_MUL_UINT1(a, b, c) HANDLE_ERROR(mpn_mul_uint1(a, b, c))
#define SHIFT_LEFT_WORDS(a, b, c) HANDLE_ERROR(shift_left_words(a, b, c))

__attribute__((unused)) static void
mpn_poison(struct mpn *a)
{
#if !defined(NDEBUG)
        memset(a->d + a->n, 0xaa, (a->max - a->n) * sizeof(*a->d));
#endif
}

static int
mpn_alloc(struct mpn *a, mp_size_t max_digits)
{
        if (max_digits <= a->max) {
                return 0;
        }
        if (max_digits > MP_SIZE_MAX / sizeof(*a->d)) {
                return EOVERFLOW;
        }
        void *p = realloc(a->d, sizeof(*a->d) * max_digits);
        if (p == NULL) {
                return ENOMEM;
        }
        a->d = p;
        a->max = max_digits;
        mpn_poison(a);
        return 0;
}

void
mpn_init(struct mpn *a)
{
        a->n = 0;
        a->max = 0;
        a->d = NULL;
}

void
mpn_clear(struct mpn *a)
{
        free(a->d);
}

static bool
is_normal(const struct mpn *a)
{
        if (a->n == 0) {
                return true;
        }
        if (a->d[a->n - 1] == 0) {
                return false;
        }
        mp_size_t i;
        for (i = 0; i < a->n; i++) {
                if (a->d[i] < 0) {
                        return false;
                }
                if (a->d[i] > COEFF_MAX) {
                        return false;
                }
        }
        return true;
}

int
mpn_cmp(const struct mpn *a, const struct mpn *b)
{
        if (a->n > b->n) {
                return 1;
        }
        if (a->n < b->n) {
                return -1;
        }
        mp_size_t n = a->n;
        mp_size_t i;
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
mpn_add(struct mpn *c, const struct mpn *a, const struct mpn *b)
{
        mp_size_t n = (a->n > b->n) ? a->n : b->n;
        mp_size_t i;
        int ret;

        if (MP_SIZE_MAX - 1 < n) {
                return EOVERFLOW;
        }
        MPN_ALLOC(c, n + 1);
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
mpn_sub(struct mpn *c, const struct mpn *a, const struct mpn *b)
{
        mp_size_t n = a->n;
        mp_size_t i;
        int ret;

        assert(mpn_cmp(a, b) >= 0);
        MPN_ALLOC(c, n);
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
mul1(struct mpn *c, const struct mpn *a, coeff_t n)
{
        assert(is_normal(a));
        if (n == 0) {
                c->n = 0;
                return;
        }
        assert(a->n < c->max || a->n == 0 ||
               a->d[a->n - 1] * n <= COEFF_TYPE_MAX);
        mp_size_t i;
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
mpn_mul(struct mpn *c, const struct mpn *a, const struct mpn *b)
{
        assert(is_normal(a));
        assert(is_normal(b));
        if (a->n == 0 || b->n == 0) {
                c->n = 0;
                return 0;
        }
        MPN_DEFINE(t);
        MPN_DEFINE(a0);
        MPN_DEFINE(b0);
        int ret;
        COPY_IF(c == a, a, a0);
        COPY_IF(c == b, b, b0);
        if (a->n > MP_SIZE_MAX - 1 || b->n > MP_SIZE_MAX - 1 - a->n) {
                ret = EOVERFLOW;
                goto fail;
        }
        MPN_ALLOC(c, a->n + b->n + 1);
        MPN_ALLOC(&t, a->n + 1);
        /*
         * initialize with 0, as b->d might contain 0s, for which
         * the following logic can leave the corresponding c->d elements
         * uninitialized.
         */
        memset(c->d, 0, c->max * sizeof(*c->d));
        mul1(c, a, b->d[0]);
        assert(is_normal(c));
        mp_size_t i;
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
                mp_size_t j;
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
        mpn_clear(&t);
        mpn_clear(&a0);
        mpn_clear(&b0);
        return ret;
}

static int
div_normalize(struct mpn *a, struct mpn *b, unsigned int *kp)
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
                        if (a->n > MP_SIZE_MAX - 1) {
                                ret = EOVERFLOW;
                                goto fail;
                        }
                        MPN_ALLOC(a, a->n + 1);
                }
                mul1(a, a, (coeff_t)1 << k);
        }
        *kp = k;
        return 0;
fail:
        return ret;
}

static int
shift_left_words(struct mpn *d, const struct mpn *s, mp_size_t n)
{
        assert(d != s);
        assert(is_normal(s));
        if (s->n == 0) {
                d->n = 0;
                return 0;
        }
        int ret;
        if (n > MP_SIZE_MAX - s->n) {
                return EOVERFLOW;
        }
        MPN_ALLOC(d, s->n + n);
        mp_size_t i;
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
mpn_divrem(struct mpn *q, struct mpn *r, const struct mpn *a,
              const struct mpn *b0)
{
        MPN_DEFINE(b);
        MPN_DEFINE(b1);
        MPN_DEFINE(tmp);
        MPN_DEFINE(tmp2);
        unsigned int k;
        int ret;

        COPY_IF(q == b0 || r == b0, b0, b1);
        assert(is_normal(a));
        assert(is_normal(b0));
        assert(b0->n != 0); /* XXX report division-by-zero? */
        MPN_SET(r, a);
        if (mpn_cmp(a, b0) < 0) {
                q->n = 0;
                ret = 0;
                goto fail;
        }
        MPN_SET(&b, b0);
        ret = div_normalize(r, &b, &k);
        if (ret != 0) {
                goto fail;
        }
        mp_size_t n = b.n;
        assert(r->n >= n);
        mp_size_t m = r->n - n;
#if !defined(NDEBUG)
        /* assert(r < 2 * (BASE ** m) * b) */
        SHIFT_LEFT_WORDS(&tmp, &b, m);
        MPN_ADD(&tmp, &tmp, &tmp);
        assert(mpn_cmp(r, &tmp) < 0);
#endif
        assert(n > 0);
        assert(m < MP_SIZE_MAX);
        MPN_ALLOC(q, m + 1);
        SHIFT_LEFT_WORDS(&tmp, &b, m); /* tmp = (BASE ** m) * b */
        if (mpn_cmp(r, &tmp) >= 0) {
                q->d[m] = 1;
                q->n = m + 1;
                MPN_SUB(r, r, &tmp);
        } else {
                q->n = m;
        }
        for (; m > 0; m--) {
                mp_size_t j = m - 1;
#if !defined(NDEBUG) && 0
                /* assert(r < (BASE ** (j + 1)) * b) */
                SHIFT_LEFT_WORDS(&tmp, &b, j + 1);
                assert(mpn_cmp(r, &b) < 0);
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
                MPN_MUL_UINT1(&tmp2, &tmp, q_j); /* tmp2 = q_j * tmp */
                while (mpn_cmp(r, &tmp2) < 0) {
                        q_j--;
                        MPN_SUB_NOFAIL(&tmp2, &tmp2, &tmp);
                }
                MPN_SUB_NOFAIL(r, r, &tmp2);
                q->d[j] = q_j;
        }
        ret = 0;
        assert(is_normal(q));
        assert(is_normal(r));
        assert(mpn_cmp(r, &b) < 0);
        if (k > 0 && r->n != 0) {
                /* r = r / (2 ** k) */
                MPN_SET_UINT1(&tmp, (coeff_t)1 << k);
                MPN_DIVREM(r, &tmp2, r, &tmp);
                assert(tmp2.n == 0); /* should be an exact division */
        }
        assert(is_normal(q));
        assert(is_normal(r));
fail:
        mpn_clear(&b);
        mpn_clear(&b1);
        mpn_clear(&tmp);
        mpn_clear(&tmp2);
        return ret;
}

int
mpn_rootint(struct mpn *s, const struct mpn *m, unsigned int k)
{
        assert(mpn_cmp(m, &g_zero) > 0);
        assert(k >= 2);
        MPN_DEFINE(m1);
        MPN_DEFINE(bk);
        MPN_DEFINE(bk_minus_1);
        MPN_DEFINE(u);
        MPN_DEFINE(tmp);
        MPN_DEFINE(unused);
        int ret;
        COPY_IF(s == m, m, m1);
        MPN_SET_UINT(&bk, k);
        MPN_SET_UINT(&bk_minus_1, k - 1);
        MPN_SET(&u, m);
        do {
                MPN_SET(s, &u);
                MPN_MUL(&u, &u, &bk_minus_1);
                /* tmp = m / (s ^ (k - 1)) */
                MPN_SET(&tmp, m);
                unsigned int i;
                for (i = 0; i < k - 1; i++) {
                        MPN_DIVREM(&tmp, &unused, &tmp, s);
                }
                MPN_ADD(&u, &u, &tmp);
                MPN_DIVREM(&u, &unused, &u, &bk);
        } while (mpn_cmp(&u, s) < 0);
        ret = 0;
fail:
        mpn_clear(&bk);
        mpn_clear(&bk_minus_1);
        mpn_clear(&u);
        mpn_clear(&tmp);
        mpn_clear(&unused);
        mpn_clear(&m1);
        return ret;
}

int
mpn_powint(struct mpn *s, const struct mpn *m, unsigned int k)
{
        MPN_DEFINE(m1);
        unsigned int i;
        int ret;
        COPY_IF(s == m, m, m1);
        MPN_SET_UINT(s, 1);
        for (i = 0; i < k; i++) {
                MPN_MUL(s, s, m);
        }
        ret = 0;
fail:
        mpn_clear(&m1);
        return ret;
}

int
mpn_set(struct mpn *d, const struct mpn *s)
{
        int ret;
        MPN_ALLOC(d, s->n);
        mp_size_t i;
        for (i = 0; i < s->n; i++) {
                d->d[i] = s->d[i];
        }
        d->n = s->n;
fail:
        return ret;
}

__attribute__((unused)) static int
mpn_set_uint1(struct mpn *a, coeff_t v)
{
        assert(v <= COEFF_MAX);
        if (v == 0) {
                a->n = 0;
                return 0;
        }
        int ret;
        MPN_ALLOC(a, 1);
        a->d[0] = v;
        a->n = 1;
fail:
        return ret;
}

int
mpn_set_uint(struct mpn *a, uintmax_t v)
{
#if defined(BASE)
        if (v <= COEFF_MAX) {
                return mpn_set_uint1(a, v);
        }
        int ret;
        MPN_DEFINE(t);
        MPN_DEFINE(bb);
        mpn_set_zero(a);       /* a = 0 */
        MPN_SET_UINT1(&bb, 1); /* bb = 1 */
        while (true) {
                coeff_t d = v % BASE;
                MPN_SET_UINT1(&t, d);
                MPN_MUL(&t, &t, &bb);
                MPN_ADD(a, a, &t); /* a += d * bb */
                v /= BASE;
                if (v == 0) {
                        break;
                }
                MPN_MUL(&bb, &bb, &g_base); /* bb *= BASE */
        }
fail:
        mpn_clear(&t);
        mpn_clear(&bb);
        return ret;
#else
        assert(v <= COEFF_MAX);
        return mpn_set_uint1(a, v);
#endif
}

__attribute__((unused)) static int
mpn_to_uint1(const struct mpn *a, coeff_t *vp)
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
mpn_to_uint(const struct mpn *a, uintmax_t *vp)
{
#if COEFF_MAX >= UINTMAX_MAX
        coeff_t c;
        int ret = mpn_to_uint1(a, &c);
        if (ret == 0) {
                *vp = c;
        }
        return ret;
#else
        uintmax_t v = 0;
        mp_size_t i;
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

static int
mpn_mul_uint1(struct mpn *d, const struct mpn *a, coeff_t b)
{
        assert(b <= COEFF_MAX);
        int ret;
        MPN_ALLOC(d, a->n + 1);
        mul1(d, a, b);
fail:
        return ret;
}

int
mpn_is_zero(const struct mpn *a)
{
        assert(is_normal(a));
        return a->n == 0;
}

void
mpn_set_zero(struct mpn *a)
{
        a->n = 0;
        assert(mpn_is_zero(a));
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

static int
mpn_from_str_base(struct mpn *a, const struct mpn *base,
                     const char *p)
{
        size_t n = strlen(p);
        int ret;

        MPN_DEFINE(tmp);
        a->n = 0; /* a = 0 */
        size_t i;
        for (i = 0; i < n; i++) {
                MPN_MUL(&tmp, a, base); /* tmp = a * base */
                unsigned int x;
                ret = digit_from_chr(p[i], &x);
                if (ret != 0) {
                        goto fail;
                }
                MPN_SET_UINT(a, x); /* a = digit */
                if (mpn_cmp(a, base) >= 0) {
                        ret = EINVAL;
                        goto fail;
                }
                MPN_ADD(a, a, &tmp); /* a = a + tmp */
        }
        ret = 0;
        assert(is_normal(a));
fail:
        mpn_clear(&tmp);
        return ret;
}

int
mpn_from_str(struct mpn *a, const char *p)
{
#if BASE == 10
        size_t n = strlen(p);
        int ret;
        MPN_ALLOC(a, n);
        mp_size_t i;
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
        return mpn_from_str_base(a, &g_ten, p);
#endif
}

int
mpn_from_hex_str(struct mpn *a, const char *p)
{
        return mpn_from_str_base(a, &g_16, p);
}

static size_t
estimate_ndigits(const struct mpn *a)
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
mpn_to_str_impl(char *p, size_t sz, const struct mpn *a,
                   const struct mpn *base)
{
        MPN_DEFINE(q);
        MPN_DEFINE(r);
        int ret;

        mp_size_t n = sz;
        MPN_SET(&q, a);
        p[--n] = 0;
        do {
                assert(n > 0);
                MPN_DIVREM(&q, &r, &q, base);
                uintmax_t u;
                MPN_TO_UINT(&r, &u);
                char ch = digit_chr(u);
                p[--n] = ch;
        } while (q.n != 0);
        if (n > 0) {
                memmove(p, &p[n], sz - n);
        }
        ret = 0;
fail:
        mpn_clear(&q);
        mpn_clear(&r);
        return ret;
}

char *
mpn_to_str(const struct mpn *a)
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
        mp_size_t i;
        for (i = 0; i < a->n; i++) {
                p[i] = a->d[a->n - i - 1] + '0';
        }
        p[i] = 0;
        return p;
#else
        if (mpn_to_str_impl(p, sz, a, &g_ten)) {
                free(p);
                return NULL;
        }
        return p;
#endif
}

static size_t
estimate_ndigits_hex(const struct mpn *a)
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
mpn_to_hex_str(const struct mpn *a)
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
        if (mpn_to_str_impl(p, sz, a, &g_16)) {
                free(p);
                return NULL;
        }
        return p;
}

void
mpn_str_free(char *p)
{
        free(p);
}

#if defined(TEST)
/* tests */
#include <stdio.h>

static void
print_mpn(const char *heading, const struct mpn *a)
{
        assert(is_normal(a));
        char *p = mpn_to_str(a);
        printf("%s%s\n", heading, p);
        mpn_str_free(p);
}

int
gcd(struct mpn *c, const struct mpn *a0, const struct mpn *b0)
{
        const struct mpn *a = a0;
        const struct mpn *b = b0;
        MPN_DEFINE(q);
        struct mpn t[3];
        unsigned int i;
        int ret;

        for (i = 0; i < 3; i++) {
                mpn_init(&t[i]);
        }

        i = 0;
        while (1) {
                // print_mpn("a  =", a);
                // print_mpn("b  =", b);
                MPN_DIVREM(&q, &t[i], a, b);
                // print_mpn("a/b=", &q);
                // print_mpn("a%b=", &t[i]);
                if (t[i].n == 0) {
                        break;
                }
                a = b;
                b = &t[i];
                i = (i + 1) % 3;
        };
        MPN_SET(c, b);
        assert(is_normal(c));
        ret = 0;
fail:
        mpn_clear(&q);
        for (i = 0; i < 3; i++) {
                mpn_clear(&t[i]);
        }
        return ret;
}

int
fixed_point_sqrt(void)
{
        const char *scale_str = "1000000000000";
        MPN_DEFINE(scale);
        MPN_DEFINE(t);
        int ret;
        MPN_FROM_STR(&scale, scale_str);
        unsigned int i;
        for (i = 1; i < 256; i++) {
                MPN_SET_UINT(&t, i);
                MPN_MUL(&t, &t, &scale);
                MPN_MUL(&t, &t, &scale);
                MPN_ROOTINT(&t, &t, 2);
                char *p = mpn_to_str(&t);
                if (p == NULL) {
                        ret = ENOMEM;
                        goto fail;
                }
                printf("sqrt(%3u) * %s = %s\n", i, scale_str, p);
                mpn_str_free(p);
        }
fail:
        mpn_clear(&scale);
        mpn_clear(&t);
        return ret;
}

#include "../sha-2/sha512_table.h"

int
sha2table(void)
{
        unsigned int primes[80];
        unsigned int n = 0;
        unsigned int x = 2; /* first prime */

        /* generate primes */
        do {
                unsigned int i;
                for (i = 0; i < n; i++) {
                        if ((x % primes[i]) == 0) {
                                goto next;
                        }
                }
                primes[n++] = x;
        next:
                x++;
        } while (n < 80);
        assert(primes[63] == 311);

        /*
         * calculate tables for sha-512, using fixed point arithmetic
         * with S=2^128.
         * maybe 128 is a bit excessive.
         */
        MPN_DEFINE(scale);
        MPN_DEFINE(scale2);
        MPN_DEFINE(unused);
        MPN_DEFINE(t);
        MPN_DEFINE(q);
        MPN_DEFINE(r);
        int ret;
        MPN_FROM_STR(&scale,
                        "340282366920938463463374607431768211456"); /* 2^128 */
        MPN_FROM_STR(&scale2, "18446744073709551616"); /* 2^(128-64) */
        unsigned int i;
        for (i = 0; i < 8; i++) {
                /* sqrt of prime */
                MPN_SET_UINT(&t, primes[i]);
                MPN_MUL(&t, &t, &scale);
                MPN_MUL(&t, &t, &scale);
                MPN_ROOTINT(&t, &t, 2);
                /* take the first 64 bits of the fraction part */
                /* q = (t % scale) / scale2 */
                MPN_DIVREM(&unused, &r, &t, &scale);
                MPN_DIVREM(&q, &unused, &r, &scale2);
                char *p = mpn_to_hex_str(&q);
                if (p == NULL) {
                        ret = ENOMEM;
                        goto fail;
                }
                printf("H[%2u] = frac(sqrt(%3u)) = %s\n", i, primes[i], p);
                mpn_str_free(p);
                /* verify with the pre-computed value */
                uintmax_t x;
                MPN_TO_UINT(&q, &x);
                assert(H[i] == x);
        }
        for (i = 0; i < 80; i++) {
                /* cbrt of prime */
                MPN_SET_UINT(&t, primes[i]);
                MPN_MUL(&t, &t, &scale);
                MPN_MUL(&t, &t, &scale);
                MPN_MUL(&t, &t, &scale);
                MPN_ROOTINT(&t, &t, 3);
                /* take the first 64 bits of the fraction part */
                MPN_DIVREM(&unused, &r, &t, &scale);
                MPN_DIVREM(&q, &unused, &r, &scale2);
                char *p = mpn_to_hex_str(&q);
                if (p == NULL) {
                        ret = ENOMEM;
                        goto fail;
                }
                printf("K[%2u] = frac(cbrt(%3u)) = %s\n", i, primes[i], p);
                /* verify with the pre-computed value */
                mpn_str_free(p);
                uintmax_t x;
                MPN_TO_UINT(&q, &x);
                assert(K[i] == x);
        }
fail:
        mpn_clear(&scale);
        mpn_clear(&scale2);
        mpn_clear(&unused);
        mpn_clear(&t);
        mpn_clear(&q);
        mpn_clear(&r);
        return ret;
}

static void
test_str_roundtrip(const char *str)
{
        MPN_DEFINE(a);
        int ret = mpn_from_str(&a, str);
        assert(ret == 0);
        char *p = mpn_to_str(&a);
        printf("expected %s\n", str);
        printf("actual   %s\n", p);
        assert(!strcmp(p, str));
        mpn_str_free(p);
        mpn_clear(&a);
}

static void
test_hex_str_roundtrip(const char *str)
{
        MPN_DEFINE(a);
        int ret = mpn_from_hex_str(&a, str);
        assert(ret == 0);
        char *p = mpn_to_hex_str(&a);
        printf("expected %s\n", str);
        printf("actual   %s\n", p);
        assert(!strcmp(p, str));
        mpn_str_free(p);
        mpn_clear(&a);
}

int
factorial(struct mpn *a, const struct mpn *n)
{
        MPN_DEFINE(c);
        int ret;
        MPN_SET_UINT1(a, 1);
        MPN_SET(&c, n);
        while (!mpn_is_zero(&c)) {
                MPN_MUL(a, a, &c);
                MPN_SUB_NOFAIL(&c, &c, &g_one);
        }
fail:
        mpn_clear(&c);
        return ret;
}

int
check_factorial(const struct mpn *a, const struct mpn *n)
{
        MPN_DEFINE(c);
        MPN_DEFINE(q);
        MPN_DEFINE(r);
        int ret;
        MPN_SET(&q, a);
        MPN_SET(&c, n);
        while (!mpn_is_zero(&c)) {
                MPN_DIVREM(&q, &r, &q, &c);
                assert(mpn_is_zero(&r));
                MPN_SUB_NOFAIL(&c, &c, &g_one);
        }
        assert(mpn_cmp(&q, &g_one) == 0);
fail:
        mpn_clear(&c);
        mpn_clear(&r);
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
        unsigned int num = 1000;
        printf("calculating %u!...\n", num);
        MPN_DEFINE(a);
        MPN_DEFINE(n);
        int ret;
        MPN_SET_UINT(&n, num);
        uint64_t start_time = timestamp();
        ret = factorial(&a, &n);
        if (ret != 0) {
                goto fail;
        }
        uint64_t end_time = timestamp();
        printf("took %.03f sec\n",
               (double)(end_time - start_time) / 1000000000);
        {
                printf("checking %u!...\n", num);
                uint64_t start_time = timestamp();
                ret = factorial(&a, &n);
                if (ret != 0) {
                        goto fail;
                }
                check_factorial(&a, &n);
                uint64_t end_time = timestamp();
                printf("took %.03f sec\n",
                       (double)(end_time - start_time) / 1000000000);
        }
#if 0
        char *ap = mpn_to_str(&a);
        if (ap == NULL) {
                goto fail;
        }
        char *np = mpn_to_str(&n);
        if (np == NULL) {
                mpn_str_free(ap);
                goto fail;
        }
        printf("%s! = %s\n", np, ap);
        mpn_str_free(ap);
        mpn_str_free(np);
#endif
fail:
        mpn_clear(&a);
        mpn_clear(&n);
        printf("ret %d\n", ret);
        return ret;
}

void
assert_eq(const struct mpn *a, const char *str)
{
        char *p = mpn_to_str(a);
        assert(p != NULL);
        if (strcmp(p, str)) {
                printf("unexpected value\n");
                printf("    actual  : %s\n", p);
                printf("    expected: %s\n", str);
                abort();
        }
        mpn_str_free(p);
}

int
main(void)
{
        const char *a_str =
                "12409715069012348970189741096590126450986902431123456";
        const char *b_str =
                "21434109785019758904721590874321400983729087987654";
        struct mpn a;
        struct mpn b;
        struct mpn c;
        struct mpn s;
        struct mpn d;
        struct mpn prod;
        struct mpn q;
        struct mpn r;
        struct mpn tmp;
        struct mpn tmp2;
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
                MPN_DEFINE(q);
                MPN_DEFINE(r);
                struct mpn a = MPN_INITIALIZER(3, 8, 0, 9);
                struct mpn b = MPN_INITIALIZER(2, 2, 9);
                print_mpn("dividend ", &a);
                print_mpn("divisor  ", &b);
                ret = mpn_divrem(&q, &r, &a, &b);
                assert(ret == 0);
                assert_eq(&q, "9");
                assert_eq(&r, "80");
                mpn_clear(&q);
                mpn_clear(&r);
        }
        {
                MPN_DEFINE(q);
                MPN_DEFINE(r);
                struct mpn a = MPN_INITIALIZER(3, 0, 1, 8);
                struct mpn b = MPN_INITIALIZER(2, 9, 9);
                print_mpn("dividend ", &a);
                print_mpn("divisor  ", &b);
                ret = mpn_divrem(&q, &r, &a, &b);
                assert(ret == 0);
                assert_eq(&q, "8");
                assert_eq(&r, "18");
                mpn_clear(&q);
                mpn_clear(&r);
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

        mpn_init(&a);
        mpn_init(&b);
        mpn_init(&c);
        mpn_init(&s);
        mpn_init(&d);
        mpn_init(&prod);
        mpn_init(&q);
        mpn_init(&r);
        mpn_init(&tmp);
        mpn_init(&tmp2);
        assert(mpn_cmp(&a, &a) == 0);
        assert(mpn_cmp(&b, &b) == 0);
        assert(mpn_cmp(&a, &b) == 0);
        assert(mpn_cmp(&b, &a) == 0);
        mpn_from_str(&a, a_str);
        assert(mpn_cmp(&a, &b) > 0);
        assert(mpn_cmp(&b, &a) < 0);
        mpn_from_str(&b, b_str);
        assert(mpn_cmp(&a, &a) == 0);
        assert(mpn_cmp(&b, &b) == 0);
        assert(mpn_cmp(&a, &b) > 0);
        assert(mpn_cmp(&b, &a) < 0);
        {
                char *p;
                p = mpn_to_hex_str(&a);
                assert(!strcmp(
                        p, "212b125567c1932ed6f400b9e883b365e3a365bac800"));
                p = mpn_to_hex_str(&b);
                assert(!strcmp(p,
                               "eaa72b959fd1535970dea1d15024b7c1325a43fc6"));
        }

        ret = mpn_from_str(&tmp, "1a");
        assert(ret == EINVAL);
        ret = mpn_from_hex_str(&tmp, "1ag");
        assert(ret == EINVAL);

        ret = mpn_set_uint(&tmp, COEFF_MAX);
        assert(ret == 0);
        ret = mpn_add(&tmp, &tmp, &g_one);
        assert(ret == 0);
        assert(!mpn_cmp(&g_base, &tmp));

        ret = mpn_set_uint(&tmp, 0);
        assert(ret == 0);
        assert_eq(&tmp, "0");
        ret = mpn_set_uint(&tmp, 9);
        assert(ret == 0);
        assert_eq(&tmp, "9");
        ret = mpn_set_uint(&tmp, 10);
        assert(ret == 0);
        assert_eq(&tmp, "10");
        ret = mpn_set_uint(&tmp, 11);
        assert(ret == 0);
        assert_eq(&tmp, "11");
        ret = mpn_set_uint(&tmp, 100);
        assert(ret == 0);
        assert_eq(&tmp, "100");

        /* add and sub */
        mpn_add(&s, &a, &b);
        assert(mpn_cmp(&s, &a) > 0);
        assert(mpn_cmp(&s, &b) > 0);
        mpn_sub(&d, &s, &a);
        assert(mpn_cmp(&d, &b) == 0);
        mpn_sub(&d, &s, &g_zero);
        assert(mpn_cmp(&d, &s) == 0);
        mpn_sub(&d, &s, &s);
        assert(mpn_cmp(&d, &g_zero) == 0);

        /* mul */
        ret = mpn_from_str(&c, "111398900663392315947914998");
        assert(ret == 0);
        ret = mpn_mul(&prod, &c, &c);
        assert(ret == 0);
        assert_eq(&prod,
                  "12409715069012348970189741000142675791423583433340004");
        ret = mpn_mul(&prod, &g_one, &c);
        assert(ret == 0);
        print_mpn("c     = ", &c);
        print_mpn("1 * c = ", &prod);
        assert(mpn_cmp(&prod, &c) == 0);
        ret = mpn_mul(&prod, &g_one, &g_one);
        assert(ret == 0);
        assert(mpn_cmp(&prod, &g_one) == 0);
        ret = mpn_mul(&prod, &c, &g_one);
        assert(ret == 0);
        assert(mpn_cmp(&prod, &c) == 0);
        ret = mpn_mul(&prod, &c, &c);
        assert(ret == 0);
        assert_eq(&prod,
                  "12409715069012348970189741000142675791423583433340004");

        ret = mpn_mul(&prod, &a, &b);
        assert(ret == 0);
        ret = mpn_divrem(&q, &r, &prod, &a);
        assert(ret == 0);
        assert(mpn_cmp(&q, &b) == 0);
        assert(mpn_cmp(&r, &g_zero) == 0);
        ret = mpn_divrem(&q, &r, &prod, &b);
        assert(ret == 0);
        assert(mpn_cmp(&q, &a) == 0);
        assert(mpn_cmp(&r, &g_zero) == 0);

        ret = mpn_from_hex_str(&tmp, "100000000000000000000000000000000");
        assert(ret == 0);
        ret = mpn_from_str(&tmp2,
                              "340282366920938463463374607431768211456");
        assert(ret == 0);
        assert(mpn_cmp(&tmp, &tmp2) == 0);
        int i;
        for (i = 1; i < 10; i++) {
                ret = mpn_set_uint(&tmp, i);
                assert(ret == 0);
                mpn_poison(&tmp);
                ret = mpn_mul(&tmp, &tmp, &tmp2);
                assert(ret == 0);
                {
                        char *p = mpn_to_hex_str(&tmp);
                        assert(p != NULL);
                        printf("p %s\n", p);
                        assert(!strcmp(p + 1,
                                       "00000000000000000000000000000000"));
                        mpn_str_free(p);
                }
        }

        /* divrem */
        ret = mpn_divrem(&q, &r, &a, &g_one);
        assert(ret == 0);
        assert(mpn_cmp(&q, &a) == 0);
        assert(mpn_cmp(&r, &g_zero) == 0);
        ret = mpn_divrem(&q, &r, &a, &b);
        assert(ret == 0);
        ret = mpn_mul(&tmp, &q, &b);
        assert(ret == 0);
        ret = mpn_add(&tmp, &tmp, &r);
        assert(ret == 0);
        assert(mpn_cmp(&tmp, &a) == 0);

        ret = mpn_divrem(&q, &r, &b, &a);
        assert(ret == 0);
        assert(mpn_cmp(&q, &g_zero) == 0);
        assert(mpn_cmp(&r, &b) == 0);

        ret = mpn_divrem(&q, &r, &a, &a);
        assert(ret == 0);
        assert(mpn_cmp(&q, &g_one) == 0);
        assert(mpn_cmp(&r, &g_zero) == 0);

        ret = mpn_divrem(&q, &r, &b, &b);
        assert(ret == 0);
        assert(mpn_cmp(&q, &g_one) == 0);
        assert(mpn_cmp(&r, &g_zero) == 0);

        fixed_point_sqrt();
        sha2table();

        /* rootint */
        unsigned int k;
        for (k = 2; k < 100; k++) {
                uint64_t start_time = timestamp();
                print_mpn("a                            = ", &a);
                printf("k                            = %u\n", k);
                ret = mpn_rootint(&q, &a, k);
                assert(ret == 0);
                print_mpn("rootint(a, k)                = ", &q);
                ret = mpn_powint(&tmp, &q, k);
                assert(ret == 0);
                print_mpn("powint(rootint(a, k),     k) = ", &tmp);
                assert(mpn_cmp(&tmp, &a) <= 0);
                ret = mpn_add(&tmp, &q, &g_one);
                assert(ret == 0);
                ret = mpn_powint(&tmp, &tmp, k);
                assert(ret == 0);
                print_mpn("powint(rootint(a, k) + 1, k) = ", &tmp);
                assert(mpn_cmp(&tmp, &a) > 0);
                uint64_t end_time = timestamp();
                printf("took %.03f sec\n",
                       (double)(end_time - start_time) / 1000000000);
        }

        ret = gcd(&tmp, &a, &b);
        assert(ret == 0);
        print_mpn("a        = ", &a);
        print_mpn("b        = ", &b);
        print_mpn("gcd(a,b) = ", &tmp);

        ret = mpn_from_str(
                &a, "533509908571101979294464811598952141168153495025132870832"
                    "519126598141168533509908571101979294464811598952141168");
        assert(ret == 0);
        ret = mpn_from_str(
                &b, "533509908571101979294464811598952141168533509908571101979"
                    "294464811598952141168533509908571101979294464811598952141"
                    "168533509908571101979294464811598952141168533509908571101"
                    "979294464811598952141168000");
        assert(ret == 0);
        ret = gcd(&tmp, &a, &b);
        assert(ret == 0);
        print_mpn("a        = ", &a);
        print_mpn("b        = ", &b);
        print_mpn("gcd(a,b) = ", &tmp);
        ret = mpn_from_str(&tmp2, "1975308624");
        assert(ret == 0);
        assert(mpn_cmp(&tmp, &tmp2) == 0);

        mpn_clear(&a);
        mpn_clear(&b);
        mpn_clear(&c);
        mpn_clear(&s);
        mpn_clear(&d);
        mpn_clear(&prod);
        mpn_clear(&q);
        mpn_clear(&r);
        mpn_clear(&tmp);
        mpn_clear(&tmp2);

        bench();
}
#endif /* TEST */
