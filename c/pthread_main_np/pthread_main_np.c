/*
 * note: the expected values in assertions are taken from macOS.
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static int ret_ctor_101;

__attribute__((constructor(101))) static void
foo(void)
{
        ret_ctor_101 = pthread_main_np();
}

static void *
thread_func(void *vp)
{
        int ret;
        ret = pthread_main_np();
        printf("pthread_main_np=%d (non-main thread)\n", ret);
        assert(ret == 0);
        return NULL;
}

int
main(int argc, char **argv)
{
        printf("pthread_main_np=%d (constructor(101))\n", ret_ctor_101);
        assert(ret_ctor_101 == 1);

        int ret;
        ret = pthread_main_np();
        printf("pthread_main_np=%d (main)\n", ret);
        assert(ret == 1);

        pthread_t t;
        ret = pthread_create(&t, NULL, thread_func, NULL);
        if (ret != 0) {
                fprintf(stderr, "pthread_create failed witd %d\n", ret);
                exit(1);
        }
        void *value;
        ret = pthread_join(t, &value);
        if (ret != 0) {
                fprintf(stderr, "pthread_create failed witd %d\n", ret);
                exit(1);
        }

        ret = pthread_main_np();
        printf("pthread_main_np=%d (main)\n", ret);
        assert(ret == 1);
}
