#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpq.h"

void
mpq_init(struct mpq *a)
{
        mpz_init(&a->numer);
        mpz_init(&a->denom);
}

void
mpq_clear(struct mpq *a)
{
        mpz_clear(&a->numer);
        mpz_clear(&a->denom);
}

int
mpq_cmp(const struct mpq *a, const struct mpq *b)
{
        assert(mpq_is_normal(a));
        assert(mpq_is_normal(b));
        MPQ_DEFINE(c);
        int ret = mpq_sub(&c, a, b);
        assert(ret == 0); /* XXX what to do? */
        int result = mpz_cmp_zero(&c.numer);
        mpq_clear(&c);
        return result;
}

int
mpq_add(struct mpq *c, const struct mpq *a, const struct mpq *b)
{
        MPQ_DEFINE(a0);
        MPQ_DEFINE(b0);
        int ret;
        MPQ_COPY_IF(c == a, a, a0);
        MPQ_COPY_IF(c == b, b, b0);
        MPZ_MUL(&c->numer, &a->numer, &b->denom);
        MPZ_MUL(&c->denom, &a->denom, &b->numer);
        MPZ_ADD(&c->numer, &c->numer, &c->denom);
        MPZ_MUL(&c->denom, &a->denom, &b->denom);
        ret = mpq_reduce(c);
fail:
        mpq_clear(&a0);
        mpq_clear(&b0);
        return ret;
}

int
mpq_sub(struct mpq *c, const struct mpq *a, const struct mpq *b)
{
        MPQ_DEFINE(a0);
        MPQ_DEFINE(b0);
        int ret;
        MPQ_COPY_IF(c == a, a, a0);
        MPQ_COPY_IF(c == b, b, b0);
        MPZ_MUL(&c->numer, &a->numer, &b->denom);
        MPZ_MUL(&c->denom, &a->denom, &b->numer);
        MPZ_SUB(&c->numer, &c->numer, &c->denom);
        MPZ_MUL(&c->denom, &a->denom, &b->denom);
        ret = mpq_reduce(c);
fail:
        mpq_clear(&a0);
        mpq_clear(&b0);
        return ret;
}

int
mpq_mul(struct mpq *c, const struct mpq *a, const struct mpq *b)
{
        MPQ_DEFINE(a0);
        MPQ_DEFINE(b0);
        int ret;
        MPQ_COPY_IF(c == a, a, a0);
        MPQ_COPY_IF(c == b, b, b0);
        MPZ_MUL(&c->numer, &a->numer, &b->numer);
        MPZ_MUL(&c->denom, &a->denom, &b->denom);
        ret = mpq_reduce(c);
fail:
        mpq_clear(&a0);
        mpq_clear(&b0);
        return ret;
}

int
mpq_div(struct mpq *c, const struct mpq *a, const struct mpq *b)
{
        MPQ_DEFINE(a0);
        MPQ_DEFINE(b0);
        int ret;
        MPQ_COPY_IF(c == a, a, a0);
        MPQ_COPY_IF(c == b, b, b0);
        MPZ_MUL(&c->numer, &a->numer, &b->denom);
        MPZ_MUL(&c->denom, &a->denom, &b->numer);
        ret = mpq_reduce(c);
fail:
        mpq_clear(&a0);
        mpq_clear(&b0);
        return ret;
}

int
mpq_set(struct mpq *d, const struct mpq *s)
{
        int ret;
        MPZ_SET(&d->numer, &s->numer);
        MPZ_SET(&d->denom, &s->denom);
fail:
        return ret;
}

int
mpq_reduce(struct mpq *a)
{
        assert(mpz_is_normal(&a->numer));
        assert(mpz_is_normal(&a->denom));
        if (a->denom.sign) {
                a->denom.sign = false;
                a->numer.sign = !a->numer.sign;
        }
        MPN_DEFINE(c);
        MPN_DEFINE(r);
        int ret;
        MP_HANDLE_ERROR(mpn_gcd(&c, &a->denom.uint, &a->numer.uint));
        MPN_DIVREM(&a->denom.uint, &r, &a->denom.uint, &c);
        assert(mpn_is_zero(&r));
        MPN_DIVREM(&a->numer.uint, &r, &a->numer.uint, &c);
        assert(mpn_is_zero(&r));
        assert(mpq_is_normal(a));
        ret = 0;
fail:
        mpn_clear(&c);
        mpn_clear(&r);
        return ret;
}

#include <stdio.h>

bool
mpq_is_normal(const struct mpq *a)
{
        if (!mpz_is_normal(&a->numer)) {
                fprintf(stderr, "abnormal numer\n");
                return false;
        }
        if (!mpz_is_normal(&a->denom)) {
                fprintf(stderr, "abnormal denom\n");
                return false;
        }
        if (a->denom.sign) {
                fprintf(stderr, "neg denom\n");
                return false;
        }
        if (mpz_cmp_zero(&a->denom) == 0) {
                fprintf(stderr, "zero denom\n");
                return false;
        }
        if (mpn_is_zero(&a->numer.uint) &&
            mpn_cmp(&a->denom.uint, &g_one) != 0) {
                return false;
        }
        bool result;
        MPN_DEFINE(c);
        int ret = mpn_gcd(&c, &a->denom.uint, &a->numer.uint);
        assert(ret == 0); /* XXX what to do? */
        result = mpn_cmp(&c, &g_one) == 0;
        mpn_clear(&c);
        return result;
}

int
mpq_from_str(struct mpq *a, const char *p, size_t sz)
{
        if (sz == 0) {
                return EINVAL;
        }
        if (memchr(p, 0, sz)) {
                return EINVAL;
        }
        if (*p == '-') {
                p++;
                sz--;
                a->numer.sign = true;
        } else {
                a->numer.sign = false;
        }
        a->denom.sign = false;
        int ret;
        const char *slash = memchr(p, '/', sz);
        if (slash) {
                const char *denom_str = slash + 1;
                size_t denom_str_len = p + sz - denom_str;
                MPN_FROM_STR(&a->denom.uint, denom_str, denom_str_len);
                if (mpn_is_zero(&a->denom.uint)) {
                        return EINVAL;
                }
                sz = slash - p;
        } else {
                MPN_SET(&a->denom.uint, &g_one);
        }
        MPN_FROM_STR(&a->numer.uint, p, sz);
        if (a->numer.sign && mpn_is_zero(&a->numer.uint)) {
                ret = EINVAL;
                goto fail;
        }
        ret = mpq_reduce(a);
fail:
        return ret;
}

int
mpq_from_strz(struct mpq *a, const char *p)
{
        return mpq_from_str(a, p, strlen(p));
}

size_t
mpq_estimate_str_size(const struct mpq *a)
{
        assert(mpq_is_normal(a));
        size_t sz = a->numer.sign;
        /* XXX check overflow */
        sz += mpn_estimate_str_size(&a->numer.uint);
        if (mpn_cmp(&a->denom.uint, &g_one) != 0) {
                sz += 1; /* '/' */
                sz += mpn_estimate_str_size(&a->denom.uint);
        }
        return sz;
}

int
mpq_to_str_into_buf(char *p, size_t sz, const struct mpq *a, size_t *szp)
{
        assert(mpq_is_normal(a));
        const char *p0 = p;
        const char *ep = p + sz;
        int ret;
        if (a->numer.sign) {
                *p++ = '-';
        }
        size_t asz;
        MP_HANDLE_ERROR(mpn_to_dec_str_into_buf(p, ep - p, &a->numer.uint, &asz));
        p += asz;
        if (mpn_cmp(&a->denom.uint, &g_one) != 0) {
                *p++ = '/';
                MP_HANDLE_ERROR(mpn_to_dec_str_into_buf(p, ep - p, &a->denom.uint,
                                                     &asz));
                p += asz;
        }
        assert(p <= ep);
        *szp = p - p0;
        ret = 0;
fail:
        return ret;
}

char *
mpq_to_strz(const struct mpq *a)
{
        /* XXX check overflow */
        size_t sz = mpq_estimate_str_size(a) + 1;
        char *p = malloc(sz);
        if (p == NULL) {
                return NULL;
        }
        if (mpq_to_str_into_buf(p, sz, a, &sz)) {
                free(p);
                return NULL;
        }
        p[sz] = 0;
        return p;
}

void
mpq_str_free(char *p)
{
        free(p);
}
