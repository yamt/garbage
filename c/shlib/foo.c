#include <assert.h>
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
 * https://github.com/emscripten-core/emscripten/issues/19861
 *
 * for some reasons, emscriptens complains on the following block.
 *
 * error: undefined symbol: weak_func2 (referenced by root reference (e.g.
 * compiled C/C++ code)) warning: To disable errors for undefined symbols use
 * `-sERROR_ON_UNDEFINED_SYMBOLS=0` warning: _weak_func2 may need to be added
 * to EXPORTED_FUNCTIONS if it arrives from a system library
 */

#if !defined(__EMSCRIPTEN__) || 1
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

const char *var_to_override = "var_to_override foo";

const char *
func_to_override()
{
        return "func_to_override foo";
}

extern int (*ptr_to_func_in_foo)(int);
int (*ptr_to_func_in_bar)(int) = func_in_bar;

__attribute__((constructor(50))) static void
ctor(void)
{
        printf("this is %s @ %s\n", __func__, __FILE__);

        /* note: ctor in foo and bar relies on the other's reloctaion */
        int n;
        n = ptr_to_func_in_foo(1000);
        printf("func_in_foo returned %d (2005 for two-level namespace, 2004 for flat namespace)\n", n);
        assert(n == 2004 || n == 2005);
        n = ptr_to_func_in_bar(1000);
        printf("func_in_bar returned %d (1005 for two-level namespace, 1004 for flat namespace)\n", n);
        assert(n == 1004 || n == 1005);
}
