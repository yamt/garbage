
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mp.h"

static coeff_t
dig(const struct bigint *a, unsigned int i)
{
        if (i < a->n) {
                return a->d[i];
        }
        return 0;
}

int
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
        coeff_t c = a + b + carry_in;
        *carry_out = c >= BASE;
        return c % BASE;
}

int
bigint_add(const struct bigint *a, const struct bigint *b, struct bigint *c)
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
        coeff_t c = a - b - carry_in;
        if (c < 0) {
                *carry_out = 1;
                return c + BASE;
        }
        *carry_out = 0;
        return c;
}

int
bigint_sub(const struct bigint *a, const struct bigint *b, struct bigint *c)
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
mul1(const struct bigint *a, coeff_t n, struct bigint *c)
{
        assert(a->n < c->max);
        if (n == 0) {
                c->n = 0;
                return;
        }
        unsigned int i;
        coeff_t carry = 0;
        for (i = 0; i < a->n; i++) {
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
bigint_mul(const struct bigint *a, const struct bigint *b, struct bigint *c)
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
        mul1(a, b->d[0], c);
        assert(is_normal(c));
        unsigned int i;
        for (i = 1; i < b->n; i++) {
                if (b->d[i] == 0) {
                        continue;
                }
                mul1(a, b->d[i], &t);
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
        return 0;
}

char *
bigint_to_str(const struct bigint *a)
{
#if BASE != 10
#error notyet
#endif
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
        struct bigint zero;
        int ret;

        bigint_init(&a);
        bigint_init(&b);
        bigint_init(&s);
        bigint_init(&d);
        bigint_init(&prod);
        bigint_init(&zero);
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
        bigint_add(&a, &b, &s);
        assert(bigint_cmp(&s, &a) > 0);
        assert(bigint_cmp(&s, &b) > 0);
        char *p = bigint_to_str(&s);
        printf("result: %s\n", p);
        assert(!strcmp(
                p, "12431149178797368729094462687464447851970631519111110"));
        bigint_str_free(p);
        bigint_sub(&s, &a, &d);
        assert(bigint_cmp(&d, &b) == 0);
        bigint_sub(&s, &zero, &d);
        assert(bigint_cmp(&d, &s) == 0);
        bigint_sub(&s, &s, &d);
        assert(bigint_cmp(&d, &zero) == 0);
        ret = bigint_mul(&a, &b, &prod);
        assert(ret == 0);
        p = bigint_to_str(&prod);
        printf("result: %s\n", p);
        assert(!strcmp(p,
                       "265991195190024741725449308469883623294768538550582409"
                       "197459854227188973713824150108575888873477812224"));
        bigint_str_free(p);
        bigint_clear(&a);
        bigint_clear(&b);
        bigint_clear(&s);
        bigint_clear(&d);
        bigint_clear(&prod);
}
