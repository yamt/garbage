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

        char buf[] = "hello this is data came from a wasm module\n";
        size_t written;
        written = fwrite(buf, sizeof(buf) - 1, 1, fp);
        assert(written == 1);

        int ret;
        ret = fclose(fp);
        assert(ret == 0);
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
