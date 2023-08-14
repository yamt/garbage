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

__attribute__((constructor(50))) static void
ctor(void)
{
        printf("this is %s @ %s\n", __func__, __FILE__);
}
