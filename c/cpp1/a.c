#include <errno.h>
#include <stdio.h>

#define CONCAT(a, b) CONCAT1(a, b)
#define CONCAT1(a, b) a##b

/*
 * note: variadic macro is C99
 */

#define THIRD(tup) THIRD1 tup
#define THIRD1(a, b, c, ...) c

/*
 * note: an assumption here is that, an errno, if defined, is a plan
 * small number.
 *
 * for example, this does NOT work with the following definitions:
 *
 *   #define EPREM (1)
 *   #define EPREM 1000
 */

/*
 * note: probes.h is the output of:
 *
 * for x in $(seq 1 200);do printf "#define PROBE_%d x, x\n" $x;done
 */

#include "probes.h"
#define IS_ERRNO_DEFINED(a) THIRD((PROBE_##a, t, f))

#define IF(a, b, c) CONCAT(IF_CASE_, a)(b, c)
#define IF_CASE_t(b, c) b
#define IF_CASE_f(b, c) c

int
f(int x)
{
        switch (x) {
#define HANDLE(a, b)                                                          \
        case a:                                                               \
                return b;
#define ERRNO_ITEM(a, b, c) IF(IS_ERRNO_DEFINED(a), HANDLE(a, b), )
#include "errno_defs.h"
#undef ERRNO_ITEM
        }
        return -1;
}
