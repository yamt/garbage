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

#if !defined(MP_USE_BUILTIN)
#define MP_USE_BUILTIN 1
#endif

#if MP_USE_BUILTIN
#if !defined(__has_builtin)
#define __has_builtin(a) 0
#endif
#define USE_BUILTIN(a) __has_builtin(a)
#else
#define USE_BUILTIN(a) 0
#endif

#if COEFF_MAX == COEFF_TYPE_MAX
#if USE_BUILTIN(__builtin_add_overflow)
#define coeff_add_overflow(a, b, c) __builtin_add_overflow(a, b, c)
#else
static bool
coeff_add_overflow(coeff_t a, coeff_t b, coeff_t *c)
{
        *c = a + b;
        return COEFF_TYPE_MAX - a < b;
}
#endif
#if USE_BUILTIN(__builtin_sub_overflow)
#define coeff_sub_overflow(a, b, c) __builtin_sub_overflow(a, b, c)
#else
static bool
coeff_sub_overflow(coeff_t a, coeff_t b, coeff_t *c)
{
        *c = a - b;
        return a < b;
}
#endif
#endif /* COEFF_MAX == COEFF_TYPE_MAX */

#define ctassert(a) _Static_assert(a, #a)

#if defined(MP_BASE)
ctassert(COEFF_MAX == MP_BASE - 1);
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
#elif defined(MP_BASE) && 16 / MP_BASE < MP_BASE
const struct mpn g_16 = MPN_INITIALIZER(2, 16 % MP_BASE, 16 / MP_BASE);
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
#if MP_USE_ASM && defined(__x86_64__) && COEFF_MAX == UINT64_MAX
        uint64_t low;
        uint64_t high;
        __asm__ __volatile__(
                "addq %4, %1; adcq $0, %0; addq %5, %1; adcq $0, %0"
                : "=&r"(high), "=&r"(low)
                : "0"(0), "1"(a), "r"(b), "r"(carry_in)
                : "cc");
        *carry_out = high;
        return low;
#elif COEFF_MAX == COEFF_TYPE_MAX
        coeff_t t;
        coeff_t carry = 0;
        if (coeff_add_overflow(a, b, &t)) {
                carry = 1;
        }
        if (coeff_add_overflow(t, carry_in, &t)) {
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
        return c % MP_BASE;
#endif
}

static coeff_t
coeff_subc(coeff_t a, coeff_t b, coeff_t carry_in, coeff_t *carry_out)
{
#if MP_USE_ASM && defined(__x86_64__) && COEFF_MAX == UINT64_MAX
        uint64_t low;
        uint64_t high;
        __asm__ __volatile__(
                "subq %4, %1; adcq $0, %0; subq %5, %1; adcq $0, %0"
                : "=&r"(high), "=&r"(low)
                : "0"(0), "1"(a), "r"(b), "r"(carry_in)
                : "cc");
        *carry_out = high;
        return low;
#elif COEFF_MAX == COEFF_TYPE_MAX
        coeff_t t;
        coeff_t carry = 0;
        if (coeff_sub_overflow(a, b, &t)) {
                carry = 1;
        }
        if (coeff_sub_overflow(t, carry_in, &t)) {
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
                return c + MP_BASE;
        }
        *carry_out = 0;
        return c;
#endif
}

static coeff_t
coeff_mul(coeff_t *highp, coeff_t a, coeff_t b, coeff_t carry_in)
{
#if MP_USE_ASM && defined(__x86_64__) && COEFF_MAX == UINT64_MAX
        uint64_t rax;
        uint64_t rdx;
        __asm__ __volatile__("mulq %3; addq %4, %1; adcq $0, %0"
                             : "=&d"(rdx), "=&a"(rax)
                             : "a"(a), "r"(b), "r"(carry_in)
                             : "cc");
        *highp = rdx;
        return rax;
#elif UINTMAX_MAX / COEFF_MAX >= COEFF_MAX
        ctassert(UINTMAX_MAX / COEFF_MAX >= COEFF_MAX);
        assert(a <= COEFF_MAX);
        assert(b <= COEFF_MAX);
        assert(carry_in <= COEFF_MAX);
        uintmax_t prod = (uintmax_t)a * b + carry_in;
        *highp = prod / MP_BASE;
        return prod % MP_BASE;
#else
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
#if MP_USE_ASM && defined(__x86_64__) && COEFF_MAX == UINT64_MAX
        uint64_t q;
        uint64_t r; /* unused */
        __asm__ __volatile__("divq %4"
                             : "=d"(r), "=a"(q)
                             : "0"(dividend_high), "1"(dividend_low),
                               "r"(divisor)
                             : "cc");
        return q;
#elif UINTMAX_MAX / COEFF_MAX >= COEFF_MAX
        ctassert(UINTMAX_MAX / COEFF_MAX >= COEFF_MAX);
        uintmax_t q =
                ((uintmax_t)dividend_high * MP_BASE + dividend_low) / divisor;
        assert(q <= COEFF_MAX);
        return q;
#else
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

#define MPN_SET_UINT1(a, b) MP_HANDLE_ERROR(mpn_set_uint1(a, b))
#define MPN_MUL_UINT1(a, b, c) MP_HANDLE_ERROR(mpn_mul_uint1(a, b, c))

void
mpn_poison(struct mpn *a __mp_unused)
{
#if !defined(NDEBUG)
        memset(a->d + a->n, 0xaa, (a->max - a->n) * sizeof(*a->d));
#endif
}

void
print_mpn(const char *heading, const struct mpn *a)
{
        assert(mpn_is_normal(a));
        char *p = mpn_to_strz(a);
        printf("%s%s\n", heading, p);
        mpn_str_free(p);
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

static void
mpn_normalize(struct mpn *c)
{
        while (c->n > 0 && c->d[c->n - 1] == 0) {
                c->n--;
        }
        assert(mpn_is_normal(c));
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

        mpn_normalize(c);
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
mpm_shift_left_words(struct mpn *d, const struct mpn *s, mp_size_t n)
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
mpn_mul_basecase(struct mpn *c, const struct mpn *a, const struct mpn *b)
{
        assert(mpn_is_normal(a));
        assert(mpn_is_normal(b));
        if (a->n < b->n) { /* prefer larger inner loop for performance */
                const struct mpn *tmp = a;
                a = b;
                b = tmp;
        }
        if (b->n == 0) {
                c->n = 0;
                return 0;
        }
        assert(a->n != 0);
        MPN_DEFINE(t);
        MPN_DEFINE(a0);
        MPN_DEFINE(b0);
        int ret;
        MPN_COPY_IF(c == a, a, a0);
        MPN_COPY_IF(c == b, b, b0);
        if (a->n > MP_SIZE_MAX - 1 || b->n > MP_SIZE_MAX - 1 - a->n) {
                ret = EOVERFLOW;
                goto fail;
        }
        /*
         * initialize 'c' with 0, as b->d might contain 0s, for which
         * the following logic can leave the corresponding c->d elements
         * uninitialized.
         */
        mp_size_t csz = a->n + b->n + 1;
        MPN_ALLOC(c, csz);
        memset(c->d, 0, csz * sizeof(*c->d));
        MPN_ALLOC(&t, a->n + 1);
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

static void
split(struct mpn *a0, struct mpn *a1, const struct mpn *a, mp_size_t k)
{
        mpn_init(a0);
        mpn_init(a1);
        if (a->n <= k) {
                *a1 = *a;
                return;
        }
        a0->d = a->d + k;
        a0->n = a->n - k;
        a1->d = a->d;
        a1->n = k;
        mpn_normalize(a1);
        assert(mpn_is_normal(a0));
        assert(mpn_is_normal(a1));
}

int
mpn_mul_karatsuba(struct mpn *c, const struct mpn *a, const struct mpn *b)
{
        const static mp_size_t n0 = 2; /* threshold */
        assert(mpn_is_normal(a));
        assert(mpn_is_normal(b));
        if (a->n < n0 || b->n < n0) {
                return mpn_mul_basecase(c, a, b);
        }
        mp_size_t k = (a->n > b->n) ? a->n : b->n;
        if (k >= MP_SIZE_MAX / 2) {
                return mpn_mul_basecase(c, a, b);
        }
        k = (k + 1) / 2;
        MPN_DEFINE(t);
        MPN_DEFINE(t2);
        MPN_DEFINE(a_copy);
        MPN_DEFINE(b_copy);
        MPN_DEFINE(c1);
        MPN_DEFINE(c2);
        int ret;
        MPN_COPY_IF(c == a, a, a_copy);
        MPN_COPY_IF(c == b, b, b_copy);
        /* note: these mpn are aliases. do not call mpn_clear on them. */
        struct mpn a0;
        struct mpn b0;
        struct mpn a1;
        struct mpn b1;
        split(&a1, &a0, a, k);
        split(&b1, &b0, b, k);
        /* revisit: simpler to use mpz? */
        bool a_diff_sign = mpn_cmp(&a1, &a0) < 0;
        bool b_diff_sign = mpn_cmp(&b1, &b0) < 0;
        /* c2 = |a0 - a1| * |b0 - b1| */
        if (a_diff_sign) {
                MPN_SUB(&t, &a0, &a1);
        } else {
                MPN_SUB(&t, &a1, &a0);
        }
        if (b_diff_sign) {
                MPN_SUB(&t2, &b0, &b1);
        } else {
                MPN_SUB(&t2, &b1, &b0);
        }
        MPN_MUL(&c2, &t, &t2);
        /* c = a0 * b0 */
        MPN_MUL(c, &a0, &b0);
        /* c1 = a1 * b1 */
        MPN_MUL(&c1, &a1, &b1);
        /* c2 = c + c1 - a_diff_sign * b_diff_sign * c2 */
        MPN_SET(&t, c);
        MPN_ADD(&t, &t, &c1);
        if (a_diff_sign ^ b_diff_sign) {
                MPN_ADD(&c2, &t, &c2);
        } else {
                MPN_SUB(&c2, &t, &c2);
        }
        /* c = c + c2 * (base ** k) + c1 * (base ** (2 * k)) */
        MPN_SHIFT_LEFT_WORDS(&t, &c2, k);
        MPN_ADD(c, c, &t);
        MPN_SHIFT_LEFT_WORDS(&t, &c1, 2 * k);
        MPN_ADD(c, c, &t);
#if 0
        {
            struct mpn x;
            mpn_init(&x);
            ret = mpn_mul_basecase(&x, a, b);
            assert(ret == 0);
            assert(mpn_cmp(c, &x) == 0);
            mpn_clear(&x);
        }
#endif
fail:
        mpn_clear(&t);
        mpn_clear(&a_copy);
        mpn_clear(&b_copy);
        mpn_clear(&c1);
        mpn_clear(&c2);
        return ret;
}

static int
div_normalize(struct mpn *a, struct mpn *b, unsigned int *kp)
{
        assert(b->n != 0);

        unsigned int k = 0;
        int ret;
#if defined(MP_BASE)
#define HALF_MP_BASE (MP_BASE / 2)
#else
#define HALF_MP_BASE ((coeff_t)1 << (MP_BASE_BITS - 1))
#endif
        while (b->d[b->n - 1] < HALF_MP_BASE) {
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

        MPN_COPY_IF(q == b0 || r == b0, b0, b1);
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
        /* assert(r < 2 * (MP_BASE ** m) * b) */
        MPN_SHIFT_LEFT_WORDS(&tmp, &b, m);
        MPN_ADD(&tmp, &tmp, &tmp);
        assert(mpn_cmp(r, &tmp) < 0);
#endif
        assert(n > 0);
        assert(m < MP_SIZE_MAX);
        MPN_ALLOC(q, m + 1);
        MPN_SHIFT_LEFT_WORDS(&tmp, &b, m); /* tmp = (MP_BASE ** m) * b */
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
                /* assert(r < (MP_BASE ** (j + 1)) * b) */
                MPN_SHIFT_LEFT_WORDS(&tmp, &b, j + 1);
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
                MPN_SHIFT_LEFT_WORDS(&tmp, &b,
                                     j);         /* tmp = (MP_BASE ** j) * b */
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

/*
 * an integer version of Newton's method
 */
int
mpn_rootint(struct mpn *s, const struct mpn *m, unsigned int k)
{
        assert(k >= 2);
        if (m->n == 0) {
                s->n = 0;
                return 0;
        }
        MPN_DEFINE(m1);
        MPN_DEFINE(bk);
        MPN_DEFINE(bk_minus_1);
        MPN_DEFINE(u);
        MPN_DEFINE(tmp);
        MPN_DEFINE(unused);
        int ret;
        MPN_COPY_IF(s == m, m, m1);
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
        MPN_COPY_IF(s == m, m, m1);
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

__mp_unused static int
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
#if defined(MP_BASE)
        if (v <= COEFF_MAX) {
                return mpn_set_uint1(a, v);
        }
        int ret;
        MPN_DEFINE(t);
        MPN_DEFINE(bb);
        mpn_set_zero(a);       /* a = 0 */
        MPN_SET_UINT1(&bb, 1); /* bb = 1 */
        while (true) {
                coeff_t d = v % MP_BASE;
                MPN_SET_UINT1(&t, d);
                MPN_MUL(&t, &t, &bb);
                MPN_ADD(a, a, &t); /* a += d * bb */
                v /= MP_BASE;
                if (v == 0) {
                        break;
                }
                MPN_MUL(&bb, &bb, &g_base); /* bb *= MP_BASE */
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

__mp_unused static int
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
#if defined(MP_BASE)
                if (UINTMAX_MAX / MP_BASE < v) {
                        return EOVERFLOW;
                }
                v *= MP_BASE;
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

char
mp_digit_chr(unsigned int x)
{
        assert(x < 16);
        return "0123456789abcdef"[x];
}

static int
mpn_from_str_base(struct mpn *a, const struct mpn *base, const char *p,
                  size_t n)
{
        if (n == 0) {
                return EINVAL;
        }
        MPN_DEFINE(tmp);
        int ret;
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
mpn_from_str(struct mpn *a, const char *p, size_t n)
{
#if MP_BASE == 10
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
        mpn_normalize(a);
fail:
        return ret;
#else
        return mpn_from_str_base(a, &g_ten, p, n);
#endif
}

int
mpn_from_strz(struct mpn *a, const char *p)
{
        return mpn_from_str(a, p, strlen(p));
}

int
mpn_from_hex_str(struct mpn *a, const char *p, size_t sz)
{
        return mpn_from_str_base(a, &g_16, p, sz);
}

int
mpn_from_hex_strz(struct mpn *a, const char *p)
{
        return mpn_from_hex_str(a, p, strlen(p));
}

size_t
mpn_estimate_str_size_from_words(mp_size_t words)
{
        if (words == 0) {
                return 1;
        }
#if MP_BASE == 10
        return words;
#else
        /* l(10) = 2.30258509299404568401 */
        /* XXX check overflow */
        return words * MP_LOG_BASE / 2.30258509299404568401 + 1;
#endif
}

size_t
mpn_estimate_str_size(const struct mpn *a)
{
        assert(mpn_is_normal(a));
        return mpn_estimate_str_size_from_words(a->n);
}

int
mpn_to_str_into_buf(char *p, size_t sz, const struct mpn *a,
                    const struct mpn *base, size_t *szp)
{
        MPN_DEFINE(q);
        MPN_DEFINE(r);
        int ret;

        mp_size_t n = sz;
        MPN_SET(&q, a);
        do {
                assert(n > 0);
                MPN_DIVREM(&q, &r, &q, base);
                uintmax_t u;
                MPN_TO_UINT(&r, &u);
                char ch = mp_digit_chr(u);
                p[--n] = ch;
        } while (q.n != 0);
        if (n > 0) {
                memmove(p, &p[n], sz - n);
        }
        *szp = sz - n;
        ret = 0;
fail:
        mpn_clear(&q);
        mpn_clear(&r);
        return ret;
}

int
mpn_to_dec_str_into_buf(char *p, size_t sz, const struct mpn *a, size_t *szp)
{
#if MP_BASE == 10
        (void)sz;
        if (a->n == 0) {
                *p = mp_digit_chr(0);
                *szp = 1;
                return 0;
        }
        mp_size_t i;
        for (i = 0; i < a->n; i++) {
                *p++ = mp_digit_chr(a->d[a->n - i - 1]);
        }
        *szp = a->n;
        return 0;
#else
        return mpn_to_str_into_buf(p, sz, a, &g_ten, szp);
#endif
}

int
mpn_to_hex_str_into_buf(char *p, size_t sz, const struct mpn *a, size_t *szp)
{
        return mpn_to_str_into_buf(p, sz, a, &g_16, szp);
}

char *
mp_to_strz(bool sign, const struct mpn *a)
{
        assert(mpn_is_normal(a));
        size_t sz =
                sign + mpn_estimate_str_size(a) + 1; /* XXX check overflow */
        char *p0 = malloc(sz);
        if (p0 == NULL) {
                return NULL;
        }
        char *p = p0;
        if (sign) {
                *p++ = '-';
                sz--;
        }
        if (a->n == 0) {
                *p++ = '0';
                *p++ = 0;
                return p0;
        }
        if (mpn_to_dec_str_into_buf(p, sz - 1, a, &sz)) {
                free(p0);
                return NULL;
        }
        p[sz] = 0;
        return p0;
}

size_t
mpn_estimate_hex_str_size(const struct mpn *a)
{
        assert(mpn_is_normal(a));
        if (a->n == 0) {
                return 1;
        }
#if MP_BASE == 10
        return a->n;
#else
        /* l(16) = 2.77258872223978123766 */
        /* XXX check overflow */
        return a->n * MP_LOG_BASE / 2.77258872223978123766 + 1;
#endif
}

char *
mp_to_hex_strz(bool sign, const struct mpn *a)
{
        assert(mpn_is_normal(a));
        size_t sz = sign + mpn_estimate_hex_str_size(a) +
                    1; /* XXX check overflow */
        char *p0 = malloc(sz);
        if (p0 == NULL) {
                return NULL;
        }
        char *p = p0;
        if (sign) {
                *p++ = '-';
                sz--;
        }
        if (a->n == 0) {
                *p++ = '0';
                *p++ = 0;
                return p0;
        }
        if (mpn_to_hex_str_into_buf(p, sz - 1, a, &sz)) {
                free(p0);
                return NULL;
        }
        p[sz] = 0;
        return p0;
}

char *
mpn_to_strz(const struct mpn *a)
{
        return mp_to_strz(0, a);
}

char *
mpn_to_hex_strz(const struct mpn *a)
{
        return mp_to_hex_strz(0, a);
}

void
mpn_str_free(char *p)
{
        free(p);
}

int
mpn_gcd(struct mpn *c, const struct mpn *a0, const struct mpn *b0)
{
        const struct mpn *a = a0;
        const struct mpn *b = b0;
        assert(a->n != 0 || b->n != 0);
        MPN_DEFINE(q);
        struct mpn t[3];
        unsigned int i;
        int ret;

        for (i = 0; i < 3; i++) {
                mpn_init(&t[i]);
        }

        if (b->n == 0) {
                MPN_SET(c, a);
        } else {
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
        }
        assert(mpn_is_normal(c));
        ret = 0;
fail:
        mpn_clear(&q);
        for (i = 0; i < 3; i++) {
                mpn_clear(&t[i]);
        }
        return ret;
}
