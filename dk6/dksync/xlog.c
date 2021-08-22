#include <stdarg.h>
#include <stdio.h>

#include "xlog.h"

void
xlog_printf(const char *fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
}
