#include <stdio.h>
#include <stdlib.h>

#include "api.h"

int
entry(void)
{
        void *p = malloc(100);
        printf("this is a wasm module %p\n", p);

        /* call native func */
        int i = add3(10);
        printf("i = %d\n", i);
        return 0;
}
