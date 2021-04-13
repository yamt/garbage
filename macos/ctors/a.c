#include <stdio.h>
#include <stdlib.h>

__attribute__((constructor))
void
myctor(void)
{
    printf("myctor\n");
}

void
#if defined(__linux__)
_entry(void)
#else
entry(void)
#endif
{
    printf("entry\n");
    exit(0);
}

int
main(int argc, char *argv[])
{
    printf("main\n");
    exit(0);
}
