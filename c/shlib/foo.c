#include <stdio.h>

int store;

void
foo_set(int x)
{
        store = x;
}

int
foo_get()
{
        return store;
}

void (*get_foo_set_ptr())(int) { return foo_set; }

__attribute__((weak)) int func_in_bar(int n);

/*
 * for some reasons, emscriptens complains on the following block.
 *
 * error: undefined symbol: weak_func2 (referenced by root reference (e.g.
 * compiled C/C++ code)) warning: To disable errors for undefined symbols use
 * `-sERROR_ON_UNDEFINED_SYMBOLS=0` warning: _weak_func2 may need to be added
 * to EXPORTED_FUNCTIONS if it arrives from a system library
 */

#if !defined(__EMSCRIPTEN__)
__attribute__((weak)) const char *weak_func2();

const char *
call_weak_func2()
{
        return weak_func2();
}

typedef const char *(*fn)();

fn
return_weak_func2()
{
        return weak_func2;
}
#endif

int
func_in_foo(int n)
{
        printf("%s in %s called\n", __func__, __FILE__);
        return func_in_bar(n * 2);
}

const char *
get_a_value_in_bar_via_foo()
{
        extern const char *a_value_in_bar;
        return a_value_in_bar;
}

const char *a_value_in_foo = "this is a value in foo";
