#include <stdio.h>

int g;

class A {
public:
	A() { g++; }
} a;

extern "C" {
void call_saved_init_funcs(void);
}

int
main()
{
	printf("g = %d\n", g);

	call_saved_init_funcs();

	printf("g = %d\n", g);
}
