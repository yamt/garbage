#include "setjmp.h"

void
longjmp_in_lib(jmp_buf buf, int val)
{
        longjmp(buf, val);
}

int
setjmp_in_lib(void (*cb)(jmp_buf))
{
        jmp_buf buf;
        volatile int x = 100;
        if (!setjmp(buf)) {
                x++;
                cb(buf);
                return 1;
        }
        x += 7;
        return x;
}
