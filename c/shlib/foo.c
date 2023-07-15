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
