#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

void
print_float(float f)
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
        printf("%f sign %" PRIx32 " exp %" PRIu32 " frac %" PRIu32 "\n", u.f,
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
        printf("%f\n", v);
}

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
        print_float(1.0 / 0.0);
        print_float(-1.0 / 0.0);
        print_float(0.0 / 0.0);
}
