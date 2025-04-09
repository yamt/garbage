void *
g(int x)
{
        extern void *f(int);
        return f(x);
}
