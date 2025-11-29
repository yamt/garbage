#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mp.h"
#include "mpz.h"

static void
print_mpn(const char *heading, const struct mpn *a)
{
        assert(mpn_is_normal(a));
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
        assert(mpn_is_normal(c));
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

int
mpz_cmp_test(const char *a_str, const char *b_str)
{
        int ret;
        int cmp;
        MPZ_DEFINE(a);
        MPZ_DEFINE(b);
        MPZ_FROM_STR(&a, a_str);
        MPZ_FROM_STR(&b, b_str);
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
        MPZ_FROM_STR(&a, a_str);
        MPZ_FROM_STR(&b, b_str);
        MPZ_ADD(&a, &a, &b);
        MPZ_FROM_STR(&b, expected_str);
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
        MPZ_FROM_STR(&a, a_str);
        MPZ_FROM_STR(&b, b_str);
        MPZ_SUB(&a, &a, &b);
        MPZ_FROM_STR(&b, expected_str);
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
        MPZ_FROM_STR(&a, a_str);
        MPZ_FROM_STR(&b, b_str);
        MPZ_MUL(&a, &a, &b);
        MPZ_FROM_STR(&b, expected_str);
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
        MPZ_FROM_STR(&a, a_str);
        MPZ_FROM_STR(&b, b_str);
        MPZ_DIVREM(&a, &b, &a, &b);
        MPZ_FROM_STR(&q, q_str);
        MPZ_FROM_STR(&r, r_str);
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

        assert(mpz_from_str(&a, "-0") == EINVAL);
        assert(mpz_from_hex_str(&a, "-0") == EINVAL);

        MPZ_FROM_STR(&a, "123456789412093790174309174");
        MPZ_FROM_STR(&b,
                     "-41234095749035790423751234567894120937901743091741");
        MPZ_SUB(&c, &a, &b);

        p = mpz_to_str(&c);
        if (p == NULL) {
                goto fail;
        }
        printf("mpz c = %s\n", p);
        assert(!strcmp(p,
                       "41234095749035790423751358024683533031691917400915"));
        mpz_str_free(p);

        p = mpz_to_hex_str(&c);
        if (p == NULL) {
                goto fail;
        }
        printf("mpz c = %s\n", p);
        assert(!strcmp(p, "1c36a8cd37a465d0915d7008685c53f6f3df8f1b53"));
        mpz_str_free(p);

        MPZ_ADD(&c, &a, &b);
        p = mpz_to_str(&c);
        if (p == NULL) {
                goto fail;
        }
        printf("mpz c = %s\n", p);
        assert(!strcmp(p,
                       "-41234095749035790423751111111104708844111568782567"));
        mpz_str_free(p);

        p = mpz_to_hex_str(&c);
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
        ret = mpn_from_str(&tmp2, "340282366920938463463374607431768211456");
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

        mpz_test();
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
