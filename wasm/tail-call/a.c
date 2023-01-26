
#include <stdio.h>

extern int (*table[])(int *p);

int
next(int *p)
{
        char foo[] = "foo"; // consume some stack
        printf("%s %p\n", foo, __builtin_frame_address(0));
        __attribute__((musttail)) return (table[*p])(p);
}
