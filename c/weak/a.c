#include <stdio.h>

extern __attribute__((weak)) char nonexist;
void *f(void);

int
main()
{
        printf("%p\n", &nonexist);
        printf("%p\n", f());
}
