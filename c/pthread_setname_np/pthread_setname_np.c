#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__NetBSD__)
#define HAVE_SETNAME_THREAD
#define HAVE_SETNAME_ARG
#define HAVE_GETNAME
#endif

#if defined(__APPLE__)
#undef HAVE_SETNAME_THREAD
#undef HAVE_SETNAME_ARG
/*
 * while macOS doesn't have a man page of pthread_getname_np,
 * it seems to have an implemenatation.
 */
#define HAVE_GETNAME
#define PTHREAD_MAX_NAMELEN_NP 16
#endif

#if defined(HAVE_SETNAME_ARG) && !defined(HAVE_SETNAME_THREAD)
#error not implemented
#endif

int
main(int argc, char **argv)
{
#if defined(HAVE_SETNAME_ARG)
        static char foo[] = "foo";
#endif
#if defined(HAVE_GETNAME)
#if defined(HAVE_SETNAME_ARG)
        static char expected[] = "hello foo!";
#else
        static char expected[] = "hello %s!";
#endif
        char name[PTHREAD_MAX_NAMELEN_NP];
#endif
        int ret;

        pthread_t t = pthread_self();
#if defined(HAVE_SETNAME_ARG)
        ret = pthread_setname_np(t, "hello %s!", foo);
#else
        ret = pthread_setname_np(
#if defined(HAVE_SETNAME_THREAD)
                t,
#endif
                "hello %s!");
#endif
        if (ret != 0) {
                fprintf(stderr, "pthread_setname_np failed with %d\n", ret);
                exit(1);
        }
#if defined(HAVE_GETNAME)
        ret = pthread_getname_np(t, name, sizeof(name));
        if (ret != 0) {
                fprintf(stderr, "pthread_getname_np failed with %d\n", ret);
                exit(1);
        }
        printf("name: '%s'\n", name);
        assert(!strcmp(name, expected));
#else
        /* sleep forever to allow manual investigation */
        while (1) {
                sleep(1);
        }
#endif
}
