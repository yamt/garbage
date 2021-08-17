#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "api.h"

void
do_some_file_io(void)
{
        const char *name = "hoge";
        FILE *fp;

        printf("opening file %s\n", name);
        fp = fopen(name, "w");
        if (fp == NULL) {
                printf("fopen failed: %s\n", strerror(errno));
        }
        assert(fp != NULL);
}

int
main(void)
{
        void *p = malloc(100);
        printf("this is a wasm module %p\n", p);

        /* call native func */
        int i = add3(10);
        printf("i = %d\n", i);

        do_some_file_io();
        return 0;
}
