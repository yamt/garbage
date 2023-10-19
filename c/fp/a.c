#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

void
_print_float(const char *label, float f)
{
        union {
                float f;
                uint32_t u;
        } u;
        /*
         * binary 32
         *
         * sign: 1 bit
         * exponent: 8 bits
         * fraction: 23 bits
         *
         * significand/mantissa/coefficient = 24
         * exponent bias = 127
         *
         * exponent bias 127
         */
        u.f = f;
        uint32_t sign = u.u >> 31;
        uint32_t exponent = (u.u >> 23) & 0xff;
        uint32_t fraction = u.u & ((1U << 23) - 1);
        printf("%s: %#x %f sign %" PRIx32 " exp %" PRIu32 " frac %" PRIu32 "\n", label, u.u, u.f,
               sign, exponent, fraction);
        /* restore hidden/implicit bit */
        float significand = (1 << 23) | fraction;
        float v = significand;
        if (exponent > 127) {
                v *= 1U << (exponent - 127);
        }
        if (exponent < 127) {
                v /= 1UL << (127 - exponent);
        }
        v /= (1 << (24 - 1));
        if (sign) {
                v = -v;
        }
        printf("%f isnan=%u isinf=%u\n", v, (int)isnan(f), (int)isinf(f));
}

#define print_float(a) _print_float(#a, a)

int
main()
{
        print_float(1);
        print_float(-1);
        print_float(100);
        print_float(-100);
        print_float(INT32_MAX);
        print_float(INT32_MIN);
        print_float(0.0);
        print_float(-0.0f);
        print_float(1.0 / 0.0); /* inf */
        print_float(INFINITY);
        print_float(-1.0 / 0.0); /* -inf */
        print_float(-INFINITY);
        print_float(0.0 / 0.0); /* nan */
        print_float(-0.0 / 0.0); /* nan */
        print_float(NAN);
        float inf = 1.0 / 0.0;
        float minus_inf = -1.0 / 0.0;
        float nan = 0.0 / 0.0;
        print_float(inf);
        print_float(minus_inf);
        print_float(nan);
        print_float(sqrt(inf));
        /*
         * Note: x86 produces NaN with sign=1 while ARM produces
         * the one with sign=0
         */
        print_float(sqrt(minus_inf));
        print_float(sqrt(-1.0));

        union {
                float f;
                uint32_t u;
        } u;
        u.u = 0xffffffff;
        print_float(u.f);
}
