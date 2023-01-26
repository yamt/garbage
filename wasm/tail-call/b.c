
#include <stdio.h>

int next(int *p);
int f1(int *p);

int (*table[])(int *p) = {
        f1,
};

int
f1(int *p)
{
        char bar[] = "bar"; // consume some stack
        printf("%s %p\n", bar, __builtin_frame_address(0));
        __attribute__((musttail)) return next(p);
}
