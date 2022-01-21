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

        time_t clock = ts.tv_sec;
        struct tm tm;
        char buf[sizeof("0000-00-00 00-00-00")];
        strftime(buf, sizeof(buf), "%F %T", localtime_r(&clock, &tm));

        va_start(ap, fmt);
        fprintf(stderr, "%s (%ju.%09ld): ", buf, (uintmax_t)ts.tv_sec,
                ts.tv_nsec);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
}
