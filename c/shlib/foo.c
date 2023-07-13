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
