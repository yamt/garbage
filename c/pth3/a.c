#define _GNU_SOURCE
#include <assert.h>
#include <pthread.h>
#if defined(__linux__)
#include <sched.h>
#endif
#include <stdio.h>
#include <unistd.h>

void *
pthread_child(void *vp)
{
        printf("%s %p\n", __func__, (void *)pthread_self());
        return NULL;
}

#if defined(__linux__)
int
clone_child(void *vp)
{
        /*
         * as this thread is created by clone, neither printf or
         * pthread_self below are expected to work.
         *
         * on my environment, (ubuntu 20.04 amd64)
         * the output is something like the following.
         *
         *   main: main 0x7f01cfb2c740
         *   main: pthread_child 0x7f01cfb2b700
         *   pthread_child 0x7f01cfb2b700
         *   clone_child 0x7f01cfb2c740
         *
         * note that main thread and this thread have the same pthread id.
         * it isn't surprising as this thread is not managed by the pthread
         * library. (thus the TLS used for pthread id is probably not set up.)
         */
        printf("%s %p\n", __func__, (void *)pthread_self());
        return 0;
}

static char stack[1024 * 1024];
#endif

int
main(void)
{
        printf("%s: main %p\n", __func__, (void *)pthread_self());

        pthread_t t;
        int ret;
        ret = pthread_create(&t, NULL, pthread_child, NULL);
        assert(ret == 0);
        printf("%s: pthread_child %p\n", __func__, (void *)t);
        void *v;
        ret = pthread_join(t, &v);
        assert(ret == 0);

#if defined(__linux__)
        /*
         * this code is broken. do not copy-and-paste.
         * see the comment in clone_child.
         */
        ret = clone(clone_child, stack + sizeof(stack),
                    CLONE_SIGHAND | CLONE_VM | CLONE_FS | CLONE_FILES |
                            CLONE_THREAD,
                    NULL);
        assert(ret != -1);
        sleep(1); /* wait for clone_child finishing */
#endif
}
