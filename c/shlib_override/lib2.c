#include <stdio.h>
#include <stdlib.h>

void
inc_mem_impl(int *p)
{
        fprintf(stderr, "an unexpected version of %s was called\n", __func__);
        abort();
}
