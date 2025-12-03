/* building this module with NDEBUG doesn't make much sense */
#undef NDEBUG

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mp.h"
#include "mpq.h"
#include "mpz.h"

#undef __up_unused

void
gcd_test1(const char *a_str, const char *b_str,
          const char *gcd_str __mp_unused)
{
        MPN_DEFINE(a);
        MPN_DEFINE(b);
        MPN_DEFINE(c);
        int ret;
        MPN_FROM_STRZ(&a, a_str);
        MPN_FROM_STRZ(&b, b_str);
        if (!mpn_is_zero(&b)) {
                MP_HANDLE_ERROR(mpn_gcd(&c, &b, &b)); /* test a == b case */
                assert(mpn_cmp(&c, &b) == 0);
        }
        MP_HANDLE_ERROR(mpn_gcd(&c, &a, &b));
        MP_HANDLE_ERROR(mpn_gcd(&a, &a, &b)); /* test c == a case */
        assert(mpn_cmp(&c, &a) == 0);
        MPN_FROM_STRZ(&a, a_str);             /* restore a */
        MP_HANDLE_ERROR(mpn_gcd(&b, &a, &b)); /* test c == b case */
        assert(mpn_cmp(&c, &b) == 0);
        char *p = mpn_to_strz(&c);
        assert(p != NULL);
        assert(!strcmp(p, gcd_str));
        mpn_str_free(p);
        ret = 0;
fail:
        assert(ret == 0);
        mpn_clear(&a);
        mpn_clear(&b);
        mpn_clear(&c);
}

void
gcd_test(void)
{
        gcd_test1("1", "1", "1");
        gcd_test1("1000", "100", "100");
        gcd_test1("7", "13", "1");
        gcd_test1("0", "1", "1");
        gcd_test1("1", "0", "1");
        gcd_test1("58904372341087431207431289790321749873299017192600951093245"
                  "19507623409815940690179120301560415605160516",
                  "56490581724309865140982103444447895567102345750625084972590"
                  "73298790842390165490276901846901862",
                  "2");
}

int
fixed_point_sqrt(void)
{
        const char *scale_str = "1000000000000";
        MPN_DEFINE(scale);
        MPN_DEFINE(t);
        int ret;
        MPN_FROM_STRZ(&scale, scale_str);
        unsigned int i;
        for (i = 1; i < 256; i++) {
                MPN_SET_UINT(&t, i);
                MPN_MUL(&t, &t, &scale);
                MPN_MUL(&t, &t, &scale);
                MPN_ROOTINT(&t, &t, 2);
                char *p = mpn_to_strz(&t);
                if (p == NULL) {
                        ret = ENOMEM;
                        goto fail;
                }
                printf("sqrt(%3u) * %s = %s\n", i, scale_str, p);
                /* validate some well known values */
#define E(n, sqrt_str)                                                        \
        case n:                                                               \
                assert(!strncmp(p, sqrt_str, strlen(sqrt_str)));              \
                break
                switch (i) {
                        E(1, "1000000000000");
                        E(2, "141421356");
                        E(3, "17320508");
                        E(4, "2000000000000");
                        E(5, "22360679");
                        E(6, "2449489");
                        E(7, "264575");
                        E(8, "2828427");
                        E(9, "3000000000000");
                        E(10, "31622");
                }
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
        MPN_FROM_STRZ(&scale,
                      "340282366920938463463374607431768211456"); /* 2^128 */
        MPN_FROM_STRZ(&scale2, "18446744073709551616"); /* 2^(128-64) */
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
                char *p = mpn_to_hex_strz(&q);
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
                char *p = mpn_to_hex_strz(&q);
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

int
mpz_cmp_test(const char *a_str, const char *b_str)
{
        int ret;
        int cmp = 0;
        MPZ_DEFINE(a);
        MPZ_DEFINE(b);
        MPZ_FROM_STRZ(&a, a_str);
        MPZ_FROM_STRZ(&b, b_str);
        cmp = mpz_cmp(&a, &b);
fail:
        mpz_clear(&a);
        mpz_clear(&b);
        assert(ret == 0);
        return cmp;
}

void
mpz_add_test(const char *a_str, const char *b_str, const char *expected_str)
{
        int ret;
        MPZ_DEFINE(a);
        MPZ_DEFINE(b);
        MPZ_FROM_STRZ(&a, a_str);
        MPZ_FROM_STRZ(&b, b_str);
        MPZ_ADD(&a, &a, &b);
        MPZ_FROM_STRZ(&b, expected_str);
        assert(mpz_cmp(&a, &b) == 0);
fail:
        mpz_clear(&a);
        mpz_clear(&b);
        assert(ret == 0);
}

void
mpz_sub_test(const char *a_str, const char *b_str, const char *expected_str)
{
        int ret;
        MPZ_DEFINE(a);
        MPZ_DEFINE(b);
        MPZ_FROM_STRZ(&a, a_str);
        MPZ_FROM_STRZ(&b, b_str);
        MPZ_SUB(&a, &a, &b);
        MPZ_FROM_STRZ(&b, expected_str);
        assert(mpz_cmp(&a, &b) == 0);
fail:
        mpz_clear(&a);
        mpz_clear(&b);
        assert(ret == 0);
}

void
mpz_mul_test(const char *a_str, const char *b_str, const char *expected_str)
{
        int ret;
        MPZ_DEFINE(a);
        MPZ_DEFINE(b);
        MPZ_FROM_STRZ(&a, a_str);
        MPZ_FROM_STRZ(&b, b_str);
        MPZ_MUL(&a, &a, &b);
        MPZ_FROM_STRZ(&b, expected_str);
        assert(mpz_cmp(&a, &b) == 0);
fail:
        mpz_clear(&a);
        mpz_clear(&b);
        assert(ret == 0);
}

void
mpz_divrem_test(const char *a_str, const char *b_str, const char *q_str,
                const char *r_str)
{
        int ret;
        MPZ_DEFINE(a);
        MPZ_DEFINE(b);
        MPZ_DEFINE(q);
        MPZ_DEFINE(r);
        MPZ_FROM_STRZ(&a, a_str);
        MPZ_FROM_STRZ(&b, b_str);
        MPZ_DIVREM(&a, &b, &a, &b);
        MPZ_FROM_STRZ(&q, q_str);
        MPZ_FROM_STRZ(&r, r_str);
        assert(mpz_cmp(&a, &q) == 0);
        assert(mpz_cmp(&b, &r) == 0);
fail:
        mpz_clear(&a);
        mpz_clear(&b);
        mpz_clear(&q);
        mpz_clear(&r);
        assert(ret == 0);
}

void
mpz_test(void)
{
        char *p;
        int ret;
        MPZ_DEFINE(a);
        MPZ_DEFINE(b);
        MPZ_DEFINE(c);

        assert(mpz_from_strz(&a, "-0") == EINVAL);
        assert(mpz_from_hex_strz(&a, "-0") == EINVAL);

        MPZ_FROM_STRZ(&a, "123456789412093790174309174");
        MPZ_FROM_HEX_STRZ(&b, "661efdf6e5237238afeb36");
        assert(mpz_cmp(&a, &b) == 0);
        MPZ_FROM_HEX_STRZ(&c, "-1c36a8cd37a465d0915d09e96a656ed381a6df301d");
        MPZ_FROM_STRZ(&b,
                      "-41234095749035790423751234567894120937901743091741");
        assert(mpz_cmp(&c, &b) == 0);
        p = mpz_to_hex_strz(&c);
        assert(p != NULL);
        assert(!strcmp(p, "-1c36a8cd37a465d0915d09e96a656ed381a6df301d"));
        mpz_str_free(p);
        p = mpz_to_strz(&c);
        assert(p != NULL);
        assert(!strcmp(p,
                       "-41234095749035790423751234567894120937901743091741"));
        mpz_str_free(p);
        assert(mpz_cmp(&a, &b) > 0);
        MPZ_SET(&c, &a);
        assert(mpz_cmp(&c, &a) == 0);
        MPZ_SET(&c, &b);
        assert(mpz_cmp(&c, &b) == 0);

        MPZ_SUB(&c, &a, &b);

        p = mpz_to_strz(&c);
        if (p == NULL) {
                goto fail;
        }
        printf("mpz c = %s\n", p);
        assert(!strcmp(p,
                       "41234095749035790423751358024683533031691917400915"));
        mpz_str_free(p);

        p = mpz_to_hex_strz(&c);
        if (p == NULL) {
                goto fail;
        }
        printf("mpz c = %s\n", p);
        assert(!strcmp(p, "1c36a8cd37a465d0915d7008685c53f6f3df8f1b53"));
        mpz_str_free(p);

        MPZ_ADD(&c, &a, &b);
        p = mpz_to_strz(&c);
        if (p == NULL) {
                goto fail;
        }
        printf("mpz c = %s\n", p);
        assert(!strcmp(p,
                       "-41234095749035790423751111111104708844111568782567"));
        mpz_str_free(p);

        p = mpz_to_hex_strz(&c);
        if (p == NULL) {
                goto fail;
        }
        printf("mpz c = %s\n", p);
        assert(!strcmp(p, "-1c36a8cd37a465d0915ca3ca6c6e89b00f6e2f44e7"));
        mpz_str_free(p);

        assert(mpz_cmp_test("0", "0") == 0);

        assert(mpz_cmp_test("-1000", "0") < 0);
        assert(mpz_cmp_test("1000", "0") > 0);

        assert(mpz_cmp_test("100", "1000") < 0);
        assert(mpz_cmp_test("100", "1000") < 0);

        assert(mpz_cmp_test("-100", "1000") < 0);
        assert(mpz_cmp_test("100", "-1000") > 0);
        assert(mpz_cmp_test("100", "1000") < 0);
        assert(mpz_cmp_test("-100", "-1000") > 0);

        mpz_add_test("-100", "100", "0");
        mpz_add_test("100", "-100", "0");
        mpz_add_test("-11", "-7", "-18");
        mpz_add_test("-11", "0", "-11");
        mpz_add_test("-11", "9", "-2");
        mpz_add_test("0", "-7", "-7");
        mpz_add_test("0", "0", "0");
        mpz_add_test("0", "9", "9");
        mpz_add_test("300", "-7", "293");
        mpz_add_test("300", "0", "300");
        mpz_add_test("300", "9", "309");

        mpz_sub_test("100", "100", "0");
        mpz_sub_test("-100", "-100", "0");
        mpz_sub_test("-11", "-7", "-4");
        mpz_sub_test("-11", "0", "-11");
        mpz_sub_test("-11", "9", "-20");
        mpz_sub_test("0", "-7", "7");
        mpz_sub_test("0", "0", "0");
        mpz_sub_test("0", "9", "-9");
        mpz_sub_test("300", "-7", "307");
        mpz_sub_test("300", "0", "300");
        mpz_sub_test("300", "9", "291");

        mpz_mul_test("-11", "-7", "77");
        mpz_mul_test("-11", "0", "0");
        mpz_mul_test("-11", "9", "-99");
        mpz_mul_test("0", "-7", "0");
        mpz_mul_test("0", "0", "0");
        mpz_mul_test("0", "9", "0");
        mpz_mul_test("300", "-7", "-2100");
        mpz_mul_test("300", "0", "0");
        mpz_mul_test("300", "9", "2700");

        mpz_divrem_test("-11", "-7", "1", "-4");
        mpz_divrem_test("-11", "9", "-1", "-2");
        mpz_divrem_test("0", "-7", "0", "0");
        mpz_divrem_test("0", "9", "0", "0");
        mpz_divrem_test("300", "-7", "-42", "6");
        mpz_divrem_test("300", "9", "33", "3");

        mpz_divrem_test("4", "7", "0", "4");
        mpz_divrem_test("4", "-7", "0", "4");
        mpz_divrem_test("-4", "7", "0", "-4");
        mpz_divrem_test("-4", "-7", "0", "-4");

        mpz_divrem_test("77", "7", "11", "0");
        mpz_divrem_test("77", "-7", "-11", "0");
        mpz_divrem_test("-77", "7", "-11", "0");
        mpz_divrem_test("-77", "-7", "11", "0");
fail:
        mpz_clear(&a);
        mpz_clear(&b);
        mpz_clear(&c);
        assert(ret == 0);
}

#define P(X)                                                                  \
        do {                                                                  \
                char *p = mpq_to_strz(&X);                                    \
                assert(p != NULL);                                            \
                printf("%s\n", p);                                            \
                mpq_str_free(p);                                              \
        } while (0)

#define P1(X)                                                                 \
        do {                                                                  \
                char *p = mpq_to_strz(&X);                                    \
                char *p2 = mpq_to_decimal_fraction_strz(&X, 64);              \
                assert(p != NULL && p2 != NULL);                              \
                printf("mpq " #X " = %s = %s\n", p, p2);                      \
                mpq_str_free(p);                                              \
                mpq_str_free(p2);                                             \
        } while (0)

void
mpq_str_test(const char *a_str, const char *frac_str, const char *dec_str)
{
        MPQ_DEFINE(a);
        MPQ_DEFINE(b);
        int ret;
        MPQ_FROM_STRZ(&a, a_str);
        char *p = mpq_to_strz(&a);
        assert(p != NULL);
        char *p2 = mpq_to_decimal_fraction_strz(&a, 5);
        assert(p2 != NULL);
        printf("%s = %s = %s\n", a_str, p, p2);
        assert(!strcmp(p, frac_str));
        assert(!strcmp(p2, dec_str));
        MPQ_FROM_STRZ(&b, p);
        assert(mpq_eq(&a, &b));
fail:
        assert(ret == 0);
        mpq_clear(&a);
        mpq_clear(&b);
}

void
mpq_add_test(const char *a_str, const char *b_str, const char *expected_str)
{
        int ret;
        MPQ_DEFINE(a);
        MPQ_DEFINE(b);
        MPQ_FROM_STRZ(&a, a_str);
        MPQ_FROM_STRZ(&b, b_str);
        MPQ_ADD(&a, &a, &b);
        printf("(%s) + (%s) = ", a_str, b_str);
        P(a);
        MPQ_FROM_STRZ(&b, expected_str);
        assert(mpq_eq(&a, &b));
fail:
        mpq_clear(&a);
        mpq_clear(&b);
        assert(ret == 0);
}

void
mpq_sub_test(const char *a_str, const char *b_str, const char *expected_str)
{
        int ret;
        MPQ_DEFINE(a);
        MPQ_DEFINE(b);
        MPQ_FROM_STRZ(&a, a_str);
        MPQ_FROM_STRZ(&b, b_str);
        MPQ_SUB(&a, &a, &b);
        printf("(%s) - (%s) = ", a_str, b_str);
        P(a);
        MPQ_FROM_STRZ(&b, expected_str);
        assert(mpq_eq(&a, &b));
fail:
        mpq_clear(&a);
        mpq_clear(&b);
        assert(ret == 0);
}

void
mpq_mul_test(const char *a_str, const char *b_str, const char *expected_str)
{
        int ret;
        MPQ_DEFINE(a);
        MPQ_DEFINE(b);
        MPQ_FROM_STRZ(&a, a_str);
        MPQ_FROM_STRZ(&b, b_str);
        MPQ_MUL(&a, &a, &b);
        printf("(%s) * (%s) = ", a_str, b_str);
        P(a);
        MPQ_FROM_STRZ(&b, expected_str);
        assert(mpq_eq(&a, &b));
fail:
        mpq_clear(&a);
        mpq_clear(&b);
        assert(ret == 0);
}

void
mpq_div_test(const char *a_str, const char *b_str, const char *expected_str)
{
        int ret;
        MPQ_DEFINE(a);
        MPQ_DEFINE(b);
        MPQ_FROM_STRZ(&a, a_str);
        MPQ_FROM_STRZ(&b, b_str);
        MPQ_DIV(&a, &a, &b);
        printf("(%s) / (%s) = ", a_str, b_str);
        P(a);
        MPQ_FROM_STRZ(&b, expected_str);
        assert(mpq_eq(&a, &b));
fail:
        mpq_clear(&a);
        mpq_clear(&b);
        assert(ret == 0);
}

int
gls(struct mpq *pi, unsigned int iterations)
{
        /*
         * pi = 4 - (4/3) + (4/5) - (4/7) + ...
         */
        MPQ_DEFINE(t);
        MPN_DEFINE(two);
        int ret;
        MPN_SET_UINT(&t.numer.uint, 4);
        MPN_SET_UINT(&t.denom.uint, 1);
        MPN_SET_UINT(&two, 2);
        MPQ_FROM_STRZ(pi, "0");
        unsigned int i;
        for (i = 0; i < iterations; i++) {
                t.numer.sign = (i % 2) != 0;
                MPQ_ADD(pi, pi, &t);
                MPN_ADD(&t.denom.uint, &t.denom.uint, &two);
        }
fail:
        mpq_clear(&t);
        mpn_clear(&two);
        return ret;
}

static int
mpq_sqrt_sub(struct mpn *s, const struct mpn *a, const struct mpn *scale)
{
        int ret;
        MPN_MUL(s, a, scale);
        MPN_MUL(s, s, scale);
        MPN_ROOTINT(s, s, 2);
fail:
        return ret;
}

static int
mpq_sqrt(struct mpq *s, const struct mpq *a, const struct mpn *scale)
{
        assert(mpq_is_normal(a));
        assert(!a->numer.sign);
        assert(&s->numer.uint != scale);
        assert(&s->denom.uint != scale);
        assert(&a->numer.uint != scale);
        assert(&a->denom.uint != scale);
        int ret;
        s->numer.sign = false;
        MP_HANDLE_ERROR(mpq_sqrt_sub(&s->numer.uint, &a->numer.uint, scale));
        s->denom.sign = false;
        MP_HANDLE_ERROR(mpq_sqrt_sub(&s->denom.uint, &a->denom.uint, scale));
        MPQ_REDUCE(s);
        ret = 0;
fail:
        return ret;
}

/* https://ja.wikipedia.org/wiki/%E3%82%AC%E3%82%A6%E3%82%B9%EF%BC%9D%E3%83%AB%E3%82%B8%E3%83%A3%E3%83%B3%E3%83%89%E3%83%AB%E3%81%AE%E3%82%A2%E3%83%AB%E3%82%B4%E3%83%AA%E3%82%BA%E3%83%A0
 */
int
gla(struct mpq *pi, unsigned int iterations)
{
        MPQ_DEFINE(a);
        MPQ_DEFINE(b);
        MPQ_DEFINE(t);
        MPQ_DEFINE(p);
        MPQ_DEFINE(oa);
        MPQ_DEFINE(tmp);
        MPQ_DEFINE(two);
        MPN_DEFINE(scale);
        int ret;
        MPN_SET_UINT(&scale, 1024);
        MPN_POWINT(&scale, &scale, iterations);
        MPQ_FROM_STRZ(&a, "1");
        /* b = 1/sqrt(2) = sqrt(1/2) */
        MPQ_FROM_STRZ(&b, "1/2");
        MP_HANDLE_ERROR(mpq_sqrt(&b, &b, &scale));
        MPQ_FROM_STRZ(&t, "1/4");
        MPQ_FROM_STRZ(&p, "1");
        MPQ_FROM_STRZ(&two, "2");
        unsigned int i;
        for (i = 0; i < iterations; i++) {
                MPQ_SET(&oa, &a);

                /* a = (a + b) / 2 */
                MPQ_ADD(&a, &a, &b);
                MPQ_DIV(&a, &a, &two);

                /* b = sqrt(oa * b) */
                MPQ_MUL(&b, &oa, &b);
                MP_HANDLE_ERROR(mpq_sqrt(&b, &b, &scale));

                /* t = t - ((oa - a) ** 2) * p */
                MPQ_SUB(&tmp, &oa, &a);
                MPQ_MUL(&tmp, &tmp, &tmp);
                MPQ_MUL(&tmp, &tmp, &p);
                MPQ_SUB(&t, &t, &tmp);

                /* p = 2 * p */
                MPQ_MUL(&p, &p, &two);
        }
        /* pi = (a + b) ** 2 / t / 4 */
        MPQ_ADD(pi, &a, &b);
        MPQ_MUL(pi, pi, pi);
        MPQ_DIV(pi, pi, &t);
        MPQ_DIV(pi, pi, &two);
        MPQ_DIV(pi, pi, &two);
fail:
        mpq_clear(&a);
        mpq_clear(&b);
        mpq_clear(&t);
        mpq_clear(&p);
        mpq_clear(&oa);
        mpq_clear(&tmp);
        mpq_clear(&two);
        mpn_clear(&scale);
        return ret;
}

void
mpq_test(void)
{
        MPQ_DEFINE(a);
        MPQ_DEFINE(b);
        MPQ_DEFINE(c);
        int ret;

        assert(mpq_from_strz(&a, "") == EINVAL);
        assert(mpq_from_strz(&a, "-") == EINVAL);
        assert(mpq_from_strz(&a, "-/1") == EINVAL);
        assert(mpq_from_strz(&a, "-1/") == EINVAL);
        assert(mpq_from_strz(&a, "1/-1") == EINVAL);
        assert(mpq_from_strz(&a, "1/0") == EINVAL);
        assert(mpq_from_strz(&a, "0/0") == EINVAL);
        assert(mpq_from_strz(&a, "-0") == EINVAL);
        assert(mpq_from_strz(&a, "-0/1") == EINVAL);

        MPQ_FROM_STRZ(&a, "1000");
        P1(a);
        MPQ_FROM_STRZ(&a, "-800");
        P1(a);
        MPQ_FROM_STRZ(&a, "800/3");
        P1(a);
        MPQ_FROM_STRZ(&a, "-800/3");
        P1(a);
        MPQ_FROM_STRZ(&a, "800/5");
        P1(a);
        MPQ_FROM_STRZ(&a, "-800/5");
        P1(a);
        MPQ_FROM_STRZ(&a, "10/4");
        P1(a);
        MPQ_FROM_STRZ(&a, "11/4");
        P1(a);
        MPQ_FROM_STRZ(&a, "12/4");
        P1(a);
        MPQ_FROM_STRZ(&b, "-4/292972481912222218180035883199999999997729728");
        P1(b);
        MPQ_FROM_STRZ(&c, "-32143124097890790785901745980719047509732143124097"
                          "8907907859017459807190475097/"
                          "235426271084290595185291909261581578442512042052952"
                          "982662498025368983106742743735655781868692067312733"
                          "45972152516430145904");
        P1(c);
        assert(mpq_eq(&b, &c));

        mpq_str_test("0/5", "0", "0");
        mpq_str_test("10/4", "5/2", "2.5");
        mpq_str_test("11/4", "11/4", "2.75");
        mpq_str_test("12/4", "3", "3");
        mpq_str_test("1/3", "1/3", "0.33333");
        mpq_str_test("-10/4", "-5/2", "-2.5");
        mpq_str_test("-11/4", "-11/4", "-2.75");
        mpq_str_test("-12/4", "-3", "-3");
        mpq_str_test("-1/3", "-1/3", "-0.33333");

        MPQ_FROM_STRZ(&a, "7/3");
        MPQ_FROM_STRZ(&b, "5/2");
        int cmp;
        MPQ_CMP(&cmp, &a, &b);
        assert(cmp < 0);

        MPQ_FROM_STRZ(&a, "1489/4451");
        MPQ_FROM_STRZ(&b, "1490/4453");
        MPQ_CMP(&cmp, &a, &b);
        assert(cmp < 0);

        mpq_add_test("-11/2", "3/5", "-49/10");
        mpq_sub_test("-11/2", "3/5", "-61/10");
        mpq_mul_test("-11/2", "3/5", "-33/10");
        mpq_div_test("-11/2", "3/5", "-55/6");

        mpq_add_test("-1", "3", "2");
        mpq_sub_test("-1", "3", "-4");
        mpq_mul_test("-1", "3", "-3");
        mpq_div_test("-1", "3", "-1/3");

        mpq_add_test("14/15", "1/15", "1");
        mpq_sub_test("14/15", "1/15", "13/15");
        mpq_mul_test("14/15", "1/15", "14/225");
        mpq_div_test("14/15", "1/15", "14");

        unsigned int i;
        for (i = 1; i < 8; i++) {
                gla(&a, i);
                printf("gla(%u) = ", i);
                P(a);
        }
        for (i = 1; i <= 128; i *= 2) {
                gls(&a, i);
                printf("gls(%u) = ", i);
                P(a);
        }
fail:
        mpq_clear(&a);
        mpq_clear(&b);
        mpq_clear(&c);
        assert(ret == 0);
}

#undef P
#undef P1

static void
test_str_roundtrip(const char *str)
{
        MPN_DEFINE(a);
        int ret __mp_unused = mpn_from_strz(&a, str);
        assert(ret == 0);
        char *p = mpn_to_strz(&a);
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
        int ret __mp_unused = mpn_from_hex_strz(&a, str);
        assert(ret == 0);
        char *p = mpn_to_hex_strz(&a);
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
        MPN_SET_UINT(a, 1);
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

static void
mul_bench(void)
{
        MPN_DEFINE(a);
        MPN_DEFINE(b);
        MPN_DEFINE(c_basecase);
        MPN_DEFINE(c_karatsuba);
        MPN_DEFINE(x);
        int ret;
        MPN_SET_UINT(&x, COEFF_MAX);
        MPN_SUB(&x, &x, &g_one);
        int pat;
        for (pat = 0; pat < 3; pat++) {
                printf("pat %d\n", pat);
                mp_size_t i;
                for (i = 1; i < 65536; i *= 2) {
                        uint64_t start_time;
                        uint64_t end_time;
                        switch (pat) {
                        case 0:
                                MPN_POWINT(&a, &x, 4);
                                MPN_POWINT(&b, &x, i);
                                break;
                        case 1:
                                MPN_POWINT(&a, &x, i);
                                MPN_POWINT(&b, &x, 4);
                                break;
                        case 2:
                                MPN_POWINT(&a, &x, i);
                                MPN_SET(&b, &a);
                                break;
                        }
                        double karatsuba;
                        double basecase;

                        /* ignore the result of the first run (j=0) */
                        int j;
                        for (j = 0; j < 2; j++) {
                                start_time = timestamp();
                                MP_HANDLE_ERROR(mpn_mul_karatsuba(&c_karatsuba,
                                                                  &a, &b));
                                end_time = timestamp();
                                karatsuba = (double)(end_time - start_time) /
                                            1000000000;

                                start_time = timestamp();
                                MP_HANDLE_ERROR(
                                        mpn_mul_basecase(&c_basecase, &a, &b));
                                end_time = timestamp();
                                basecase = (double)(end_time - start_time) /
                                           1000000000;
                        }

                        printf("mul_bench words (%zu x %zu) basecase %.05f "
                               "karatsuba %.05f "
                               "(%.05fx)\n",
                               (size_t)a.n, (size_t)b.n, basecase, karatsuba,
                               basecase == 0 ? 0 : karatsuba / basecase);
                        assert(!mpn_cmp(&c_basecase, &c_karatsuba));
                }
        }
        ret = 0;
fail:
        assert(ret == 0);
        mpn_clear(&a);
        mpn_clear(&b);
        mpn_clear(&c_basecase);
        mpn_clear(&c_karatsuba);
        mpn_clear(&x);
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
        char *ap = mpn_to_strz(&a);
        if (ap == NULL) {
                goto fail;
        }
        char *np = mpn_to_strz(&n);
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
        char *p = mpn_to_strz(a);
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
        int ret __mp_unused;

#if BASE_BITS == 64 && 0
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

        assert(mpn_is_normal(&g_zero));
        assert(mpn_is_normal(&g_one));
        assert(mpn_is_normal(&g_ten));
        assert(mpn_is_normal(&g_base));
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
        mpn_from_strz(&a, a_str);
        assert(mpn_cmp(&a, &b) > 0);
        assert(mpn_cmp(&b, &a) < 0);
        mpn_from_strz(&b, b_str);
        assert(mpn_cmp(&a, &a) == 0);
        assert(mpn_cmp(&b, &b) == 0);
        assert(mpn_cmp(&a, &b) > 0);
        assert(mpn_cmp(&b, &a) < 0);
        {
                char *p __mp_unused;
                p = mpn_to_hex_strz(&a);
                assert(!strcmp(
                        p, "212b125567c1932ed6f400b9e883b365e3a365bac800"));
                p = mpn_to_hex_strz(&b);
                assert(!strcmp(p,
                               "eaa72b959fd1535970dea1d15024b7c1325a43fc6"));
        }

        ret = mpn_from_strz(&tmp, "1a");
        assert(ret == EINVAL);
        ret = mpn_from_hex_strz(&tmp, "1ag");
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
        static const char tiny_int_str[] = "3141592";
        static const char tiny_int_pow2_str[] = "9869600294464";
        static const char small_int_str[] = "314159265358979323846264338327950"
                                            "2884197169399375105820974944";
        static const char small_int_pow2_str[] =
                "9869604401089358618834490999876151135313699407240790626413345"
                "654640275450817680422376413059183758982006598738098675803136";
        static const char mid_int_str[] =
                "3141592653589793238462643383279502884197169399375105820974944"
                "5923078164062862089986280348253421170679821480865132823066470"
                "9384460955058223172535940812848111745028410270193852110555964"
                "4622948954930381964428810975665933446128475648233786783165271"
                "201909145648566923460348610454326648213393607260249141273724"
                "5";
        static const char mid_int_pow2_str[] =
                "9869604401089358618834490999876151135313699407240790626413349"
                "3762200448224192052430017734037185522318240259137740231440777"
                "7234812203004672761061767798519766099039985620657563057150604"
                "1232840328780869352769342164939666571519044538735261779413820"
                "2582605816934125155920483098188732700330762666711043589508709"
                "5742169535887435990039715080989860860112642104179012804904114"
                "4740667328791473134675656141500394239273564072423804048662614"
                "1499108003005272608287443988000843591771855179456751234305773"
                "9562660179131689366471898629843845730604829968354452033250550"
                "530678850893542414981497086013878690629225401306623410190025";
        static const char large_int_str[] =
                "3141592653589793238462643383279502884197169399375105820974944"
                "5923078164062862089986280348253421170679821480865132823066470"
                "9384460955058223172535940812848111745028410270193852110555964"
                "4622948954930381964428810975665933446128475648233786783165271"
                "2019091456485669234603486104543266482133936072602491412737245"
                "8700660631558817488152092096282925409171536436789259036001133"
                "0530548820466521384146951941511609433057270365759591953092186"
                "1173819326117931051185480744623799627495673518857527248912279"
                "3818301194912983367336244065664308602139494639522473719070217"
                "9860943702770539217176293176752384674818467669405132000568127"
                "1452635608277857713427577896091736371787214684409012249534301"
                "4654958537105079227968925892354201995611212902196086403441815"
                "9813629774771309960518707211349999998372978049951059731732816"
                "0963185950244594553469083026425223082533446850352619311881710"
                "1000313783875288658753320838142061717766914730359825349042875"
                "5468731159562863882353787593751957781857780532171226806613001"
                "9278766111959092164201988";
        static const char large_int_pow2_str[] =
                "9869604401089358618834490999876151135313699407240790626413349"
                "3762200448224192052430017734037185522318240259137740231440777"
                "7234812203004672761061767798519766099039985620657563057150604"
                "1232840328780869352769342164939666571519044538735261779413820"
                "2582605816934125155920483098188732700330762666711043589508715"
                "0410032578853659527635775283792268331874508640454635412502697"
                "3729566958334227858150006365227095472490859756072669264752779"
                "0052853364522066698082641589687710573278892917469015455100692"
                "5443245703644965617253792860760600814597258922923241424004429"
                "5981361814413706777778194739658303170856632789570753407991714"
                "5231589263721144638282644328528037928503480952338995039685746"
                "0948534600901774293220579903591735782046575804193168682300219"
                "6146899270420614296963466005799840351642136543049984533721736"
                "5572404636768488762615122990270599380102994468861817162609801"
                "3087653003706015836919867628600507993646832266973156836717555"
                "8971198752975296394916315539449195483877068721130789866575590"
                "9865363656307636308806106331572828813404555390872331355933569"
                "0766808608198704031928488303454702455229594192381761854447563"
                "8904258155334564180027924022958344417699017301663203133873035"
                "5184766561265323589273514966971507662991830721824944002687562"
                "3370227691294865897597238630859504505025567734363278894100790"
                "0305370225520128960211919941393853574611724837682608377715667"
                "5284324405157005476725611043618952052031043968945214663936677"
                "1894148928654788374227320001149252155882027662238295129175502"
                "7279425922382044442058023155343241012992472705564327963726485"
                "1917760925433989030041949343786005069043157998318484908264217"
                "5523011207703743866568075543802152203739653792890342180838285"
                "0666039981382325111821878322612712781956933170143142971049740"
                "7467070621221219664131314082155568654075423401448183179012900"
                "2894484169452781480329970952173880674817992836141273826594895"
                "4197120498396607215591335428173152693230870028182089997270853"
                "6069330754570442595589039401459533054433958581052481145809073"
                "1724112268437214279885725005890989112084863152144";
        ret = mpn_from_strz(&c, tiny_int_str);
        assert(ret == 0);
        ret = mpn_mul(&prod, &c, &c);
        assert(ret == 0);
        assert_eq(&prod, tiny_int_pow2_str);
        ret = mpn_from_strz(&c, small_int_str);
        assert(ret == 0);
        ret = mpn_mul(&prod, &c, &c);
        assert(ret == 0);
        assert_eq(&prod, small_int_pow2_str);
        ret = mpn_from_strz(&c, mid_int_str);
        assert(ret == 0);
        ret = mpn_mul(&prod, &c, &c);
        assert(ret == 0);
        assert_eq(&prod, mid_int_pow2_str);
        ret = mpn_from_strz(&c, large_int_str);
        assert(ret == 0);
        ret = mpn_mul(&prod, &c, &c);
        assert(ret == 0);
        assert_eq(&prod, large_int_pow2_str);
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
        assert_eq(&prod, large_int_pow2_str);

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

        ret = mpn_from_hex_strz(&tmp, "100000000000000000000000000000000");
        assert(ret == 0);
        ret = mpn_from_strz(&tmp2, "340282366920938463463374607431768211456");
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
                        char *p = mpn_to_hex_strz(&tmp);
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

        gcd_test();
        mpz_test();
        mpq_test();
        mul_bench();
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

        ret = mpn_gcd(&tmp, &a, &b);
        assert(ret == 0);
        print_mpn("a        = ", &a);
        print_mpn("b        = ", &b);
        print_mpn("gcd(a,b) = ", &tmp);

        ret = mpn_from_strz(
                &a, "533509908571101979294464811598952141168153495025132870832"
                    "519126598141168533509908571101979294464811598952141168");
        assert(ret == 0);
        ret = mpn_from_strz(
                &b, "533509908571101979294464811598952141168533509908571101979"
                    "294464811598952141168533509908571101979294464811598952141"
                    "168533509908571101979294464811598952141168533509908571101"
                    "979294464811598952141168000");
        assert(ret == 0);
        ret = mpn_gcd(&tmp, &a, &b);
        assert(ret == 0);
        print_mpn("a        = ", &a);
        print_mpn("b        = ", &b);
        print_mpn("gcd(a,b) = ", &tmp);
        ret = mpn_from_strz(&tmp2, "1975308624");
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
