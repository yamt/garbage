#include <stdio.h>

int func();

void
fill(int *p)
{
}

int
calc(const int *p)
{
        return 1;
}

int func();

int
main()
{
        int ret = func();
        printf("func returned %d\n", ret);
        return 0;
}
