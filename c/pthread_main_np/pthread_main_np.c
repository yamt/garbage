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
        static const int is_main = 0;
        pthread_t parent = vp;
        int ret;

        if (++step > 16) {
                return NULL;
        }

        ret = pthread_main_np();
        printf("pthread_main_np=%d (step=%u, non-main thread)\n", ret, step);
        assert(is_main == ret);

        void *value;
        ret = pthread_join(parent, &value);
        if (ret != 0) {
                fprintf(stderr, "pthread_create failed with %d\n", ret);
                exit(1);
        }

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
        static const int is_main = 1;
        int ret;

        printf("pthread_main_np=%d (constructor(101))\n", ret_ctor_101);
        assert(ret_ctor_101 == 1);

        ret = pthread_main_np();
        printf("pthread_main_np=%d (main)\n", ret);
        assert(is_main == ret);
        next(is_main);
}
