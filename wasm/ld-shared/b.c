void *
g(void)
{
        extern void *f(void);
        return f();
}
