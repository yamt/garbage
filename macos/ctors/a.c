#include <stdio.h>
#include <stdlib.h>

__attribute__((constructor))
void
myctor(void)
{
    printf("myctor\n");
}

void
entry(void)
{
    printf("entry\n");
    exit(0);
}
