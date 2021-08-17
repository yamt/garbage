#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
        void *p = malloc(100);
        printf("this is a wasm module %p\n", p);
}
