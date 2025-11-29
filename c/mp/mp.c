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
static int mpn_mul_uint1(struct mpn *d, const struct mpn *a, coeff_t b);

#define MPN_SET_UINT1(a, b) HANDLE_ERROR(mpn_set_uint1(a, b))
#define MPN_MUL_UINT1(a, b, c) HANDLE_ERROR(mpn_mul_uint1(a, b, c))
#define SHIFT_LEFT_WORDS(a, b, c) HANDLE_ERROR(shift_left_words(a, b, c))

void
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

bool
mpn_is_normal(const struct mpn *a)
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
        assert(mpn_is_normal(a));
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
        assert(mpn_is_normal(c));
}

int
mpn_mul(struct mpn *c, const struct mpn *a, const struct mpn *b)
{
        assert(mpn_is_normal(a));
        assert(mpn_is_normal(b));
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
        assert(mpn_is_normal(c));
        mp_size_t i;
        for (i = 1; i < b->n; i++) {
                assert(i < b->n);
                if (b->d[i] == 0) {
                        continue;
                }
                mul1(&t, a, b->d[i]);
                assert(mpn_is_normal(&t));
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
                assert(mpn_is_normal(c));
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
        assert(mpn_is_normal(s));
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
        assert(mpn_is_normal(d));
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
        assert(mpn_is_normal(a));
        assert(mpn_is_normal(b0));
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
                SHIFT_LEFT_WORDS(&tmp, &b, j);   /* tmp = (BASE ** j) * b */
                MPN_MUL_UINT1(&tmp2, &tmp, q_j); /* tmp2 = q_j * tmp */
                while (mpn_cmp(r, &tmp2) < 0) {
                        q_j--;
                        MPN_SUB_NOFAIL(&tmp2, &tmp2, &tmp);
                }
                MPN_SUB_NOFAIL(r, r, &tmp2);
                q->d[j] = q_j;
        }
        ret = 0;
        assert(mpn_is_normal(q));
        assert(mpn_is_normal(r));
        assert(mpn_cmp(r, &b) < 0);
        if (k > 0 && r->n != 0) {
                /* r = r / (2 ** k) */
                MPN_SET_UINT1(&tmp, (coeff_t)1 << k);
                MPN_DIVREM(r, &tmp2, r, &tmp);
                assert(tmp2.n == 0); /* should be an exact division */
        }
        assert(mpn_is_normal(q));
        assert(mpn_is_normal(r));
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
        assert(mpn_is_normal(a));
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
mpn_from_str_base(struct mpn *a, const struct mpn *base, const char *p)
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
        assert(mpn_is_normal(a));
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
        assert(mpn_is_normal(a));
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
        assert(mpn_is_normal(a));
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
        assert(mpn_is_normal(a));
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
        assert(mpn_is_normal(a));
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
        assert(mpn_is_normal(a));
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
