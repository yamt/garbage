#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

double
int_to_double(uint64_t i)
{
        union {
                uint64_t i;
                double f;
        } u;
        u.i = i;
        return u.f;
}

uint64_t
double_to_int(double f)
{
        union {
                uint64_t i;
                double f;
        } u;
        u.f = f;
        return u.i;
}

/*
 * 7ff8000000000000 qnan canonical
 * 7ff0000000000000 +inf
 * fff0000000000000 -inf
 */

int
main()
{
        double snan = int_to_double(0x7ff4000000000000);
        double qnan = NAN;
        double inf = INFINITY;
        double minus_inf = -inf;

        printf("%" PRIx64 " qnan\n", double_to_int(qnan));
        printf("%" PRIx64 " snan\n", double_to_int(snan));

        printf("%" PRIx64 " ceil(snan)\n", double_to_int(ceil(snan)));
        printf("%" PRIx64 " builtin_ceil(snan)\n",
               double_to_int(__builtin_ceil(snan)));

        printf("%" PRIx64 " floor(snan)\n", double_to_int(floor(snan)));
        printf("%" PRIx64 " builtin_floor(snan)\n",
               double_to_int(__builtin_floor(snan)));

        printf("%" PRIx64 " trunc(snan)\n", double_to_int(trunc(snan)));
        printf("%" PRIx64 " builtin_trunc(snan)\n",
               double_to_int(__builtin_trunc(snan)));

        printf("%" PRIx64 " rint(snan)\n", double_to_int(rint(snan)));
        printf("%" PRIx64 " builtin_rint(snan)\n",
               double_to_int(__builtin_rint(snan)));

        printf("%" PRIx64 " nearbyint(snan)\n",
               double_to_int(nearbyint(snan)));
        printf("%" PRIx64 " builtin_nearbyint(snan)\n",
               double_to_int(__builtin_nearbyint(snan)));

        printf("%" PRIx64 " sqrt(snan)\n", double_to_int(sqrt(snan)));
        printf("%" PRIx64 " builtin_sqrt(snan)\n",
               double_to_int(__builtin_sqrt(snan)));
}
