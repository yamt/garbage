#include <assert.h>
#include <stdio.h>

int
func_in_bar(int n)
{
        printf("%s in %s called\n", __func__, __FILE__);
        return n + 5;
}

const char *func_in_main();

const char *
call_func_in_main()
{
        return func_in_main();
}

const char *
get_a_value_in_foo_via_bar()
{
        extern const char *a_value_in_foo;
        return a_value_in_foo;
}

const char *a_value_in_bar = "this is a value in bar";

int
recurse_bar(int i)
{
        extern int recurse_main(int i);
#if defined(__has_attribute)
#if __has_attribute(musttail)
#if !defined(__wasm__) || defined(__wasm_tail_call__)
        __attribute__((musttail))
#endif
#endif
#endif
        return recurse_main(i - 1);
}

const char *var_to_override = "var_to_override bar";

const char *
func_to_override()
{
        return "func_to_override bar";
}

extern int func_in_foo(int);
int (*ptr_to_func_in_foo)(int) = func_in_foo;
extern int (*ptr_to_func_in_bar)(int);

__attribute__((constructor(50))) static void
ctor(void)
{
        printf("this is %s @ %s\n", __func__, __FILE__);

        /* note: ctor in foo and bar relies on the other's reloctaion */
        int n;

        n = ptr_to_func_in_foo(1000);
        printf("func_in_foo returned %d (2005 for two-level namespace, 2004 "
               "for flat namespace)\n",
               n);
        assert(n == 2004 || n == 2005);
#if defined(__wasm__)
        assert(n == 2004);
#endif

        n = ptr_to_func_in_bar(1000);
        printf("func_in_bar returned %d (1005 for two-level namespace, 1004 "
               "for flat namespace)\n",
               n);
        assert(n == 1004 || n == 1005);
#if defined(__wasm__)
        assert(n == 1004);
#endif
}

#if defined(__wasm__)
/*
 * weak to work around:
 * https://github.com/llvm/llvm-project/issues/103592
 */
__attribute__((weak)) extern char __heap_base;
__attribute__((weak)) extern char __heap_end;
__attribute__((weak)) extern char __stack_low;
__attribute__((weak)) extern char __stack_high;

char *
bar_heap_base(void)
{
        return &__heap_base;
}

char *
bar_heap_end(void)
{
        return &__heap_end;
}

char *
bar_stack_low(void)
{
        return &__stack_low;
}

char *
bar_stack_high(void)
{
        return &__stack_high;
}
#endif
