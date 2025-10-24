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

ctassert(COEFF_MAX == BASE - 1);

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

static coeff_t
dig(const struct bigint *a, unsigned int i)
{
        if (i < a->n) {
                return a->d[i];
        }
        return 0;
}

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
                return false;
        }
        unsigned int i;
        for (i = 0; i < a->n; i++) {
                if (a->d[i] < 0) {
                        return false;
                }
                if (a->d[i] >= BASE) {
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

static coeff_t
coeff_addc(coeff_t a, coeff_t b, coeff_t carry_in, coeff_t *carry_out)
{
        ctassert(BASE * 2 < COEFF_TYPE_MAX);
        assert(0 <= carry_in);
        assert(carry_in <= 1);
        coeff_t c = a + b + carry_in;
        *carry_out = c >= BASE;
        assert(0 <= *carry_out);
        assert(*carry_out <= 1);
        return c % BASE;
}

int
bigint_add(struct bigint *c, const struct bigint *a, const struct bigint *b)
{
        unsigned int n = (a->n > b->n) ? a->n : b->n;
        unsigned int i;
        int ret;

        ret = bigint_alloc(c, n + 1);
        if (ret != 0) {
                return ret;
        }
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
}

static coeff_t
coeff_subc(coeff_t a, coeff_t b, coeff_t carry_in, coeff_t *carry_out)
{
        assert(0 <= carry_in);
        assert(carry_in <= 1);
        coeff_t c = a - b - carry_in;
        if (c < 0) {
                *carry_out = 1;
                return c + BASE;
        }
        *carry_out = 0;
        return c;
}

int
bigint_sub(struct bigint *c, const struct bigint *a, const struct bigint *b)
{
        unsigned int n = a->n;
        unsigned int i;
        int ret;

        assert(bigint_cmp(a, b) >= 0);
        ret = bigint_alloc(c, n);
        if (ret != 0) {
                return ret;
        }
        coeff_t carry = 0;
        for (i = 0; i < n; i++) {
                c->d[i] = coeff_subc(dig(a, i), dig(b, i), carry, &carry);
        }
        c->n = n;
        assert(carry == 0);
        while (c->n > 0 && c->d[c->n - 1] == 0) {
                c->n--;
        }
        return 0;
}

static void
mul1(struct bigint *c, const struct bigint *a, coeff_t n)
{
        if (n == 0) {
                c->n = 0;
                return;
        }
        assert(a->n < c->max || a->n == 0 || a->d[a->n - 1] * n < BASE);
        unsigned int i;
        coeff_t carry = 0;
        for (i = 0; i < a->n; i++) {
                ctassert(COEFF_MAX * COEFF_MAX + COEFF_MAX <= COEFF_TYPE_MAX);
                coeff_t t = a->d[i] * n + carry;
                c->d[i] = t % BASE;
                carry = t / BASE;
        }
        c->n = a->n;
        assert(c->n <= c->max);
        if (carry > 0) {
                assert(c->n < c->max);
                c->d[c->n++] = carry;
        }
}

int
bigint_mul(struct bigint *c, const struct bigint *a, const struct bigint *b)
{
        assert(is_normal(a));
        assert(is_normal(b));
        if (b->n == 0) {
                c->n = 0;
                return 0;
        }
        struct bigint t;
        int ret;
        ret = bigint_alloc(c, a->n + b->n + 1);
        if (ret != 0) {
                return ret;
        }
        bigint_init(&t);
        ret = bigint_alloc(&t, a->n + 1);
        if (ret != 0) {
                bigint_clear(&t);
                return ret;
        }
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
        bigint_clear(&t);
        return 0;
}

static int
div_normalize(struct bigint *a, struct bigint *b, unsigned int *kp)
{
        assert(b->n != 0);

        unsigned int k = 0;
        while (b->d[b->n - 1] < BASE / 2) {
                k++;
                mul1(b, b, 2);
        }
        if (k > 0) {
                if (a->n > 0) {
                        int ret = bigint_alloc(a, a->n + 1);
                        if (ret != 0) {
                                return ret;
                        }
                }
                mul1(a, a, 1 << k);
        }
        *kp = k;
        return 0;
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
        int ret = bigint_alloc(d, s->n + n);
        if (ret != 0) {
                return ret;
        }
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
}

int
bigint_divrem(struct bigint *q, struct bigint *r, const struct bigint *a,
              const struct bigint *b0)
{
        assert(q != b0);
        assert(r != b0);
        struct bigint b;
        struct bigint tmp;
        struct bigint tmp2;
        unsigned int k;
        int ret;

        assert(is_normal(a));
        assert(is_normal(b0));
        assert(b0->n != 0); /* XXX report division-by-zero? */
        bigint_init(&b);
        bigint_init(&tmp);
        bigint_init(&tmp2);
        ret = bigint_set(r, a);
        if (ret != 0) {
                goto fail;
        }
        if (bigint_cmp(a, b0) < 0) {
                q->n = 0;
                ret = 0;
                goto fail;
        }
        ret = bigint_set(&b, b0);
        if (ret != 0) {
                goto fail;
        }
        ret = div_normalize(r, &b, &k);
        if (ret != 0) {
                goto fail;
        }
        unsigned int n = b.n;
        assert(r->n >= n);
        unsigned int m = r->n - n;
        ret = bigint_alloc(q, m + 1);
        if (ret != 0) {
                goto fail;
        }
        /* tmp = (BASE ** m) * b */
        ret = shift_left_words(&tmp, &b, m);
        if (ret != 0) {
                goto fail;
        }
        int cmp = bigint_cmp(r, &tmp);
        if (cmp >= 0) {
                q->d[m] = 1;
                q->n = m + 1;
                ret = bigint_sub(r, r, &tmp);
                assert(ret == 0);
        } else {
                q->n = m;
        }
        if (m > 0) {
                unsigned int j = m - 1;
                do {
                        coeff_t q_j = (r->d[n + j] * BASE + r->d[n + j - 1]) /
                                      b.d[n - 1];
                        if (q_j > BASE - 1) {
                                q_j = BASE - 1;
                        }
                        /* tmp = (BASE ** j) * b */
                        ret = shift_left_words(&tmp, &b, j);
                        if (ret != 0) {
                                goto fail;
                        }
                        /* tmp2 = q_j * tmp */
                        ret = bigint_mul_uint(&tmp2, &tmp, q_j);
                        if (ret != 0) {
                                goto fail;
                        }
                        while (bigint_cmp(r, &tmp2) < 0) {
                                q_j--;
                                ret = bigint_sub(&tmp2, &tmp2, &tmp);
                                assert(ret == 0);
                        }
                        ret = bigint_sub(r, r, &tmp2);
                        assert(ret == 0);
                        q->d[j] = q_j;
                } while (j-- > 0);
        }
        ret = 0;
        assert(is_normal(q));
        assert(is_normal(r));
        if (k > 0 && r->n != 0) {
                /* r = r / (2 ** k) */
                ret = bigint_set_uint(&tmp, 1 << k);
                if (ret != 0) {
                        goto fail;
                }
                ret = bigint_divrem(r, &tmp2, r, &tmp);
                if (ret != 0) {
                        goto fail;
                }
                assert(tmp2.n == 0); /* should be an exact division */
        }
        assert(is_normal(q));
        assert(is_normal(r));
fail:
        bigint_clear(&b);
        bigint_clear(&tmp);
        bigint_clear(&tmp2);
        return ret;
}

int
bigint_set(struct bigint *d, const struct bigint *s)
{
        int ret = bigint_alloc(d, s->n);
        if (ret != 0) {
                return ret;
        }
        unsigned int i;
        for (i = 0; i < s->n; i++) {
                d->d[i] = s->d[i];
        }
        d->n = s->n;
        return 0;
}

int
bigint_set_uint(struct bigint *a, coeff_t v)
{
        assert(v < BASE);
        if (v == 0) {
                a->n = 0;
                return 0;
        }
        int ret = bigint_alloc(a, 1);
        if (ret != 0) {
                return ret;
        }
        a->d[0] = v;
        a->n = 1;
        return 0;
}

int
bigint_mul_uint(struct bigint *d, const struct bigint *a, coeff_t b)
{
        assert(b < BASE);
        int ret = bigint_alloc(d, a->n + 1);
        if (ret != 0) {
                return ret;
        }
        mul1(d, a, b);
        return 0;
}

int
bigint_from_str(struct bigint *a, const char *p)
{
#if BASE != 10
#error notyet
#endif
        size_t n = strlen(p);
        int ret = bigint_alloc(a, n);
        if (ret != 0) {
                return ret;
        }
        unsigned int i;
        for (i = 0; i < n; i++) {
                a->d[i] = p[n - i - 1] - '0';
        }
        a->n = n;
        if (a->d[a->n - 1] == 0) {
                a->n--;
        }
        assert(is_normal(a));
        return 0;
}

char *
bigint_to_str(const struct bigint *a)
{
#if BASE != 10
#error notyet
#endif
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
        struct bigint q;
        struct bigint t[3];
        unsigned int i;
        int ret;

        bigint_init(&q);
        for (i = 0; i < 3; i++) {
                bigint_init(&t[i]);
        }

        i = 0;
        while (1) {
                // print_bigint("a  =", a);
                // print_bigint("b  =", b);
                ret = bigint_divrem(&q, &t[i], a, b);
                if (ret != 0) {
                        goto quit;
                }
                // print_bigint("a/b=", &q);
                // print_bigint("a%b=", &t[i]);
                if (t[i].n == 0) {
                        break;
                }
                a = b;
                b = &t[i];
                i = (i + 1) % 3;
        };
        ret = bigint_set(c, b);
        assert(ret != 0 || is_normal(c));

quit:
        bigint_clear(&q);
        for (i = 0; i < 3; i++) {
                bigint_clear(&t[i]);
        }
        return ret;
}

static void
test_str_roundtrip(const char *str)
{
        struct bigint a;
        bigint_init(&a);
        int ret = bigint_from_str(&a, str);
        assert(ret == 0);
        char *p = bigint_to_str(&a);
        assert(!strcmp(p, str));
        bigint_str_free(p);
        bigint_clear(&a);
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
}
