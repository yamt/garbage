static void
x(void)
{
}

// note: making this "void f(void)" seems to make -flto=thin happy
extern void f() __attribute__((__weak__, __alias__("x")));

void
g(void)
{
        f();
}
