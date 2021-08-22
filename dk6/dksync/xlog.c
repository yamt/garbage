#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "xlog.h"

void
xlog_printf(const char *fmt, ...)
{
        struct timespec ts;
        va_list ap;

        clock_gettime(CLOCK_REALTIME, &ts);
        va_start(ap, fmt);
        fprintf(stderr, "%ju.%09ld: ", (uintmax_t)ts.tv_sec, ts.tv_nsec);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
}
