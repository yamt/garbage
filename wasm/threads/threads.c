#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PRINT_SYM(a)                                                          \
        extern unsigned char a;                                               \
        printf(#a " %p\n", &a)

#define PRINT_WEAK_SYM(a)                                                     \
        extern __attribute__((weak)) unsigned char a;                         \
        printf(#a " %p\n", &a)

#define PRINT_GLOBAL(a)                                                       \
        extern uint32_t a __attribute__((address_space(1)));                  \
        printf(#a " %" PRIx32 "\n", a)

pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_attr_t attr;
pthread_attr_t *g_attr = &attr;

#undef USE_MUTEX
#undef USE_FLOCKFILE

static void
lock(void)
{
#if defined(USE_MUTEX)
        int ret = pthread_mutex_lock(&g_lock);
        assert(ret == 0);
#else
#if defined(USE_FLOCKFILE)
        flockfile(stdout);
#endif
#endif
}

static void
unlock(void)
{
#if defined(USE_MUTEX)
        int ret = pthread_mutex_unlock(&g_lock);
        assert(ret == 0);
#else
#if defined(USE_FLOCKFILE)
        funlockfile(stdout);
#endif
#endif
}

static int
tid(pthread_t t)
{
        /*
         * XXX specific to a particular version of wasi-libc.
         * works for 16.0 and the latest main as of writing this.
         */
        const int *p = (const void *)t;
        return p[5];
}

void
print_env(void)
{
        printf("pthread_self %ju\n", (uintmax_t)pthread_self());
        printf("tid %u\n", tid(pthread_self()));
        printf("__builtin_frame_address %p\n", __builtin_frame_address(0));
        printf("__builtin_wasm_tls_base %p\n", __builtin_wasm_tls_base());
        printf("__builtin_wasm_tls_size %zu\n", __builtin_wasm_tls_size());
        printf("__builtin_wasm_tls_align %zu\n", __builtin_wasm_tls_align());

        extern void *__stack_pointer __attribute__((address_space(1)));
        printf("__stack_pointer %p\n", __stack_pointer);

#if 1 /* https://reviews.llvm.org/D135910 */
        PRINT_WEAK_SYM(__stack_low);
        PRINT_WEAK_SYM(__stack_high);
#endif

        /* https://reviews.llvm.org/D63833?id=206726 */
        PRINT_SYM(__global_base);
        PRINT_SYM(__data_end);
        PRINT_SYM(__heap_base);
#if 1 /* https://reviews.llvm.org/D136110 */
        PRINT_WEAK_SYM(__heap_end);
#endif
        PRINT_SYM(__dso_handle);

        /* https://github.com/WebAssembly/tool-conventions/blob/main/DynamicLinking.md
         */
        PRINT_GLOBAL(__memory_base);
        PRINT_GLOBAL(__table_base);

        PRINT_GLOBAL(__tls_base);

        printf("shadow stack size %zu\n", &__heap_base - &__data_end);
}

void *
start(void *vp)
{
        lock();
        printf("%s: hello %p\n", __func__, vp);
        print_env();
        unlock();
#if 0
        if ((int)vp == 0x32f) {
                printf("%s: EXIT", __func__);
                fflush(NULL);
                exit(2);
        }
#endif
        return (void *)0x123;
}

void
test_cv_timeout(void)
{
        pthread_mutex_t lock;
        pthread_cond_t cv;
        int ret;
        ret = pthread_mutex_init(&lock, NULL);
        assert(ret == 0);
        ret = pthread_cond_init(&cv, NULL);
        assert(ret == 0);

        ret = pthread_mutex_lock(&lock);
        assert(ret == 0);
        int i;
        for (i = 0; i < 3; i++) {
                struct timespec tv;
                ret = clock_gettime(CLOCK_REALTIME, &tv);
                assert(ret == 0);
                tv.tv_sec += 1;
                printf("waiting for cond\n");
                ret = pthread_cond_timedwait(&cv, &lock, &tv);
                assert(ret == ETIMEDOUT);
                printf("timedout\n");
        }
}

void *
poller(void *vp)
{
        struct pollfd pfd;
        int ret;
        pfd.fd = STDIN_FILENO;
        pfd.events = POLLIN;
        ret = poll(&pfd, 1, -1);
        assert(ret == 0);
        assert(0);
}

void
create_poll_thread(void)
{
        pthread_t t;
        int ret;
        ret = pthread_create(&t, g_attr, poller, NULL);
        assert(ret == 0);
        ret = pthread_detach(t);
        assert(ret == 0);
}

int
main(void)
{
        int ret;
        bool failed = false;
        setvbuf(stdout, NULL, _IONBF, 0);
        printf("%s: hello\n", __func__);
        print_env();

        ret = pthread_attr_init(&attr);
        assert(ret == 0);
        ret = pthread_attr_setstacksize(&attr, 4096);
        assert(ret == 0);
#if 0
        ret = pthread_attr_setguardsize(&attr, 0);
        assert(ret == 0);
#endif

        create_poll_thread();

        unsigned int n = 16;
        pthread_t t[n];
        unsigned int i;
        for (i = 0; i < n; i++) {
                lock();
                printf("%s: spawning thread %u/%u\n", __func__, i, n);
                unlock();
                ret = pthread_create(&t[i], g_attr, start,
                                     (void *)(0x321 + i));
                if (ret != 0) {
                        printf("pthread_create failed with %d\n", ret);
                        failed = true;
                        break;
                }
                lock();
                printf("%s: spawned thread %u/%u\n", __func__, i, n);
                unlock();
        }
        // pthread_exit(NULL);
        n = i;
        for (i = 0; i < n; i++) {
                void *value;
                lock();
                printf("%s: joining thread %u/%u\n", __func__, i, n);
                unlock();
                ret = pthread_join(t[i], &value);
                assert(ret == 0);
                lock();
                printf("%s: joined thread %u/%u %p\n", __func__, i, n, value);
                unlock();
        }

        print_env();

        test_cv_timeout();

        if (failed) {
                printf("failed\n");
                return 1;
        }
        printf("succeeded\n");
        //__wasi_proc_exit(0);
        return 0;
}
