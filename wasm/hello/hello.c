#include <inttypes.h>
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

_Thread_local int tls_a = 123;

void
print_env(void)
{
#if 0
        printf("pthread_self %ju\n", (uintmax_t)pthread_self());
        printf("tid %u\n", tid(pthread_self()));
#endif
        printf("__builtin_frame_address %p\n", __builtin_frame_address(0));
#if 0
        printf("__builtin_wasm_tls_base %p\n", __builtin_wasm_tls_base());
        printf("__builtin_wasm_tls_size %zu\n", __builtin_wasm_tls_size());
        printf("__builtin_wasm_tls_align %zu\n", __builtin_wasm_tls_align());
#endif

        printf("tls_a = %u, &tls_a = %p\n", tls_a, &tls_a);

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
        printf("__builtin_wasm_memory_grow(0,0) %p\n",
               (void *)__builtin_wasm_memory_grow(0, 0));

        /* https://github.com/WebAssembly/tool-conventions/blob/main/DynamicLinking.md
         */
        PRINT_GLOBAL(__memory_base);
        PRINT_GLOBAL(__table_base);

        PRINT_GLOBAL(__tls_base);

        printf("shadow stack size %zu\n", &__heap_base - &__data_end);
}

int
main(void)
{
        printf("hello\n");
        print_env();

        return 0;
}
