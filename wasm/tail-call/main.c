
#include <stdio.h>

extern int next(int *p);

int
main()
{
        int i = 0;
        printf("%d\n", next(&i));
}
