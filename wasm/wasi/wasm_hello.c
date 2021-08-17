#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
	void *p = malloc(100);
	printf("hello %p\n", p);
}
