#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char **argv)
{
        static char foo[] = "foo";
        static char expected[] = "hello foo!";
        char name[PTHREAD_MAX_NAMELEN_NP];
        int ret;

        pthread_t t = pthread_self();
        ret = pthread_setname_np(t, "hello %s!", foo);
        if (ret != 0) {
                fprintf(stderr, "pthread_setname_np failed with %d\n", ret);
                exit(1);
        }
        ret = pthread_getname_np(t, name, sizeof(name));
        if (ret != 0) {
                fprintf(stderr, "pthread_getname_np failed with %d\n", ret);
                exit(1);
        }
        printf("name: '%s'\n", name);
        assert(!strcmp(name, expected));
}
