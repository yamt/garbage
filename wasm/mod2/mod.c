#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#if 0
void *
call_indirect(void *(*fn)(void *), void *vp)
{
        return fn(vp);
}
#endif

void *
cb1(void *vp)
{
        printf("this is cb1 %p\n", vp);
        return vp;
}

void *
cb2(void *vp)
{
        printf("this is cb2 %p\n", vp);
        free(vp);
        vp = strdup("hello from cb2");
        return vp;
}

void *
f(void *vp)
{
        printf("this is a thread in wasm %p\n", vp);
        return vp;
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

        void *p2;
        p2 = call(cb1, p);
        assert(p == p2);
        p2 = call(cb2, p);
        printf("%s\n", (const char *)p2);

        pthread_t t;
        int ret;
        printf("wasm pthread_create\n");
        ret = pthread_create(&t, NULL, f, p);
        assert(ret == 0);
        void *v;
        printf("wasm pthread_join\n");
        ret = pthread_join(t, &v);
        assert(ret == 0);
        assert(v == p);

        printf("wasm done\n");
        return 0;
}
