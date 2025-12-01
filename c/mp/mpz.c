#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "mpz.h"

static int
sign_cmp(bool s1, bool s2)
{
        if (s1 == s2) {
                return 0;
        }
        if (s1) {
                return -1;
        }
        return 1;
}

static int
sign_mul(bool s1, bool s2)
{
        if (s1) {
                return !s2;
        }
        return s2;
}

void
mpz_init(struct mpz *a)
{
        a->sign = 0;
        mpn_init(&a->uint);
}

void
mpz_clear(struct mpz *a)
{
        mpn_clear(&a->uint);
}

bool
mpz_is_normal(const struct mpz *a)
{
        if (a->sign && mpn_is_zero(&a->uint)) {
                return false;
        }
        return mpn_is_normal(&a->uint);
}

int
mpz_cmp(const struct mpz *a, const struct mpz *b)
{
        assert(mpz_is_normal(a));
        assert(mpz_is_normal(b));
        int ret = sign_cmp(a->sign, b->sign);
        if (ret != 0) {
                return ret;
        }
        if (a->sign) {
                return mpn_cmp(&b->uint, &a->uint);
        }
        return mpn_cmp(&a->uint, &b->uint);
}

int
mpz_add(struct mpz *c, const struct mpz *a, const struct mpz *b)
{
        int ret;
        assert(mpz_is_normal(a));
        assert(mpz_is_normal(b));
        if (a->sign == b->sign) {
                c->sign = a->sign;
                ret = mpn_add(&c->uint, &a->uint, &b->uint);
                if (ret != 0) {
                        return ret;
                }
                c->sign = !mpn_is_zero(&c->uint) && a->sign;
                assert(mpz_is_normal(c));
                return 0;
        }
        if (mpn_cmp(&a->uint, &b->uint) >= 0) {
                ret = mpn_sub(&c->uint, &a->uint, &b->uint);
                if (ret != 0) {
                        return ret;
                }
                c->sign = !mpn_is_zero(&c->uint) && a->sign;
                assert(mpz_is_normal(c));
                return 0;
        }
        ret = mpn_sub(&c->uint, &b->uint, &a->uint);
        if (ret != 0) {
                return ret;
        }
        c->sign = !mpn_is_zero(&c->uint) && !a->sign;
        assert(mpz_is_normal(c));
        return 0;
}

int
mpz_sub(struct mpz *c, const struct mpz *a, const struct mpz *b)
{
        int ret;
        assert(mpz_is_normal(a));
        assert(mpz_is_normal(b));
        if (a->sign != b->sign) {
                c->sign = a->sign;
                ret = mpn_add(&c->uint, &a->uint, &b->uint);
                if (ret != 0) {
                        return ret;
                }
                c->sign = !mpn_is_zero(&c->uint) && a->sign;
                assert(mpz_is_normal(c));
                return 0;
        }
        if (mpn_cmp(&a->uint, &b->uint) >= 0) {
                ret = mpn_sub(&c->uint, &a->uint, &b->uint);
                if (ret != 0) {
                        return ret;
                }
                c->sign = !mpn_is_zero(&c->uint) && a->sign;
                assert(mpz_is_normal(c));
                return 0;
        }
        ret = mpn_sub(&c->uint, &b->uint, &a->uint);
        if (ret != 0) {
                return ret;
        }
        c->sign = !mpn_is_zero(&c->uint) && !a->sign;
        assert(mpz_is_normal(c));
        return 0;
}

int
mpz_mul(struct mpz *c, const struct mpz *a, const struct mpz *b)
{
        assert(mpz_is_normal(a));
        assert(mpz_is_normal(b));
        int ret = mpn_mul(&c->uint, &a->uint, &b->uint);
        if (ret != 0) {
                return ret;
        }
        c->sign = !mpn_is_zero(&c->uint) && sign_mul(a->sign, b->sign);
        assert(mpz_is_normal(c));
        return 0;
}

int
mpz_divrem(struct mpz *q, struct mpz *r, const struct mpz *a,
           const struct mpz *b)
{
        assert(mpz_is_normal(a));
        assert(mpz_is_normal(b));
        int ret = mpn_divrem(&q->uint, &r->uint, &a->uint, &b->uint);
        if (ret != 0) {
                return ret;
        }
        bool a_sign = a->sign;
        bool b_sign = b->sign;
        q->sign = !mpn_is_zero(&q->uint) && sign_mul(a_sign, b_sign);
        r->sign = !mpn_is_zero(&r->uint) && a_sign;
        assert(mpz_is_normal(q));
        assert(mpz_is_normal(r));
        return 0;
}

int
mpz_set(struct mpz *d, const struct mpz *s)
{
        d->sign = s->sign;
        int ret = mpn_set(&d->uint, &s->uint);
        assert(ret != 0 || mpz_is_normal(d));
        return ret;
}

int
mpz_from_strz(struct mpz *a, const char *p)
{
        if (*p == '-') {
                a->sign = 1;
                p++;
        } else {
                a->sign = 0;
        }
        int ret = mpn_from_strz(&a->uint, p);
        if (ret != 0) {
                return ret;
        }
        if (a->sign && mpn_is_zero(&a->uint)) {
                return EINVAL;
        }
        assert(mpz_is_normal(a));
        return 0;
}

int
mpz_from_hex_strz(struct mpz *a, const char *p)
{
        if (*p == '-') {
                a->sign = 1;
                p++;
        } else {
                a->sign = 0;
        }
        int ret = mpn_from_hex_strz(&a->uint, p);
        if (ret != 0) {
                return ret;
        }
        if (a->sign && mpn_is_zero(&a->uint)) {
                return EINVAL;
        }
        assert(mpz_is_normal(a));
        return 0;
}

char *
mpz_to_str(const struct mpz *a)
{
        return mp_to_str(a->sign, &a->uint);
}

char *
mpz_to_hex_str(const struct mpz *a)
{
        return mp_to_hex_str(a->sign, &a->uint);
}

void
mpz_str_free(char *p)
{
        free(p);
}
