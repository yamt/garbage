#include <stdarg.h>
#include <stdio.h>

int
sum(int count, ...)
{
        va_list ap;
        int n = 0;
        int i;

        va_start(ap, count);
        for (i = 0; i < count; i++) {
                n += va_arg(ap, int);
        }
        va_end(ap);
        return n;
}

int
main()
{
        printf("%d\n", sum(2, 1, 2));
        printf("%d\n", sum(5, 1, 2, 3, 4, 5));
}
