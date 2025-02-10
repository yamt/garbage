void f(void);
void (*f_p)(void) = f;

void
_start(void)
{
#if 0
        if (f_p != f) {
                __builtin_trap();
        }
#endif
        f_p();
}
