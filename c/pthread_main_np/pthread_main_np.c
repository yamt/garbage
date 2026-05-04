/*
 * note: the expected values in assertions are taken from macOS.
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned int step;
static int ret_ctor_101;

__attribute__((constructor(101))) static void
foo(void)
{
        ret_ctor_101 = pthread_main_np();
}

static void next(int) __attribute__((noreturn));

static void *
thread_func(void *vp)
{
        pthread_t parent = vp;
        int ret;
        int is_main;

        if (++step > 16) {
                return NULL;
        }

        is_main = pthread_main_np();
        printf("pthread_main_np=%d (step=%u, non-main thread)\n", is_main,
               step);
        assert(is_main == 0);

        void *value;
        ret = pthread_join(parent, &value);
        if (ret != 0) {
                fprintf(stderr, "pthread_create failed with %d\n", ret);
                exit(1);
        }

        ret = pthread_main_np();
        printf("pthread_main_np=%d (step=%u, non-main thread, after joining "
               "parent)\n",
               ret, step);
        assert(ret == is_main);

        next(is_main);
        return NULL;
}

static void
next(int is_main)
{
        pthread_t t;
        int ret;

        assert(is_main == pthread_main_np());
        ret = pthread_create(&t, NULL, thread_func, pthread_self());
        if (ret != 0) {
                fprintf(stderr, "pthread_create failed witd %d\n", ret);
                exit(1);
        }
        assert(is_main == pthread_main_np());
        pthread_exit(NULL);
}

int
main(int argc, char **argv)
{
        printf("pthread_main_np=%d (constructor(101))\n", ret_ctor_101);
        assert(ret_ctor_101 == 1);

        int is_main = pthread_main_np();
        printf("pthread_main_np=%d (main)\n", is_main);
        assert(is_main == 1);

        next(is_main);
}
