#include <stdio.h>

__attribute__((constructor))
void
myctor(void)
{
    printf("myctor\n");
}

int
entry()
{
    printf("entry\n");
    return 0;
}
