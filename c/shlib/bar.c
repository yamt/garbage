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
