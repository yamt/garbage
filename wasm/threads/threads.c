#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

#define PRINT_SYM(a)                                                          \
        extern unsigned char a;                                               \
        printf(#a " %p\n", &a)

#define PRINT_WEAK_SYM(a)                                                     \
        extern __attribute__((weak)) unsigned char a;                         \
        printf(#a " %p\n", &a)

#define PRINT_GLOBAL(a)                                                       \
        extern uint32_t a __attribute__((address_space(1)));                  \
        printf(#a " %" PRIx32 "\n", a)

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
        printf("%s: hello %p\n", __func__, vp);
        print_env();
        return (void *)0x123;
}

int
main(void)
{
        printf("%s: hello\n", __func__);
        print_env();

        pthread_t t;
        int ret;
        ret = pthread_create(&t, NULL, start, (void *)0x321);
        if (ret != 0) {
                printf("pthread_create failed with %d\n", ret);
        }
        assert(ret == 0);
        void *value;
        printf("%s: join\n", __func__);
        ret = pthread_join(t, &value);
        assert(ret == 0);
        printf("%s: joined %p\n", __func__, value);

        print_env();
        return 0;
}
