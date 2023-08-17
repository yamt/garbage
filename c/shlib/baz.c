#include <errno.h>
#include <stdio.h>
#include <string.h>

int
fn(const char *caller)
{
        printf("hi, %s. this is %s @ %s\n", caller, __func__, __FILE__);
        return strnlen(caller, 100);
}

int var = 42;

__attribute__((constructor(50))) static void
ctor(void)
{
        printf("this is %s @ %s\n", __func__, __FILE__);
        errno = 4321;
}

void *
get_printf_ptr()
{
        return printf;
}
