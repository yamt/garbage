
#include <assert.h>
#include <errno.h>
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
        struct bigint i;
        struct bigint a;
        struct bigint b;
        bigint_init(&i);
        bigint_init(&a);
        bigint_init(&b);
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
        bigint_add(&a, &b, &i);
        assert(bigint_cmp(&i, &a) > 0);
        assert(bigint_cmp(&i, &b) > 0);
        char *p = bigint_to_str(&i);
        printf("result: %s\n", p);
        assert(!strcmp(
                p, "12431149178797368729094462687464447851970631519111110"));
        bigint_str_free(p);
        bigint_clear(&i);
        bigint_clear(&a);
        bigint_clear(&b);
}
