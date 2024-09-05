void f(void);
void (*f_p)(void) = f;

void
_start(void)
{
		f_p();
}
