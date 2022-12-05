#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#define PRINT_SYM(a)                                                          \
        extern unsigned char a;                                               \
        printf(#a " %p\n", &a)

int
main(void)
{
        printf("hello\n");

        printf("__builtin_frame_address %p\n", __builtin_frame_address(0));

        extern void *__stack_pointer __attribute__((address_space(1)));
        printf("__stack_pointer %p\n", __stack_pointer);

#if 0 /* https://reviews.llvm.org/D135910 */
        PRINT_SYM(__stack_low);
        PRINT_SYM(__stack_high);
#endif

        /* https://reviews.llvm.org/D63833?id=206726 */
        PRINT_SYM(__global_base);
        PRINT_SYM(__data_end);
        PRINT_SYM(__heap_base);
#if 0 /* https://reviews.llvm.org/D136110 */
        PRINT_SYM(__heap_end);
#endif
        PRINT_SYM(__dso_handle);

        /* https://github.com/WebAssembly/tool-conventions/blob/main/DynamicLinking.md
         */
        extern uint32_t __memory_base __attribute__((address_space(1)));
        extern uint32_t __table_base __attribute__((address_space(1)));
        printf("__memory_base %" PRIx32 "\n", __memory_base);
        printf("__table_base %" PRIx32 "\n", __table_base);

        printf("shadow stack size %zu\n", &__heap_base - &__data_end);

        return 0;
}
