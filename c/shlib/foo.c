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

int func_in_bar(int n);

int
func_in_foo(int n)
{
        return func_in_bar(n * 2);
}

const char *
get_a_value_in_bar_via_foo()
{
        extern const char *a_value_in_bar;
        return a_value_in_bar;
}

const char *a_value_in_foo = "this is a value in foo";
