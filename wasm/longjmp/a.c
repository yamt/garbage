
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__wasm__)
/* for some reasons, __builtin_setjmp/__builtin_longjmp is not used */
typedef void *jmp_buf[1];
int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);
#else
typedef void *jmp_buf[5];
#define setjmp __builtin_setjmp
#define longjmp __builtin_longjmp
#endif

/*
 * note: a jmp_buf for __builtin_setjmp is of 5 words
 * https://gcc.gnu.org/onlinedocs/gcc/Nonlocal-Gotos.html
 * https://llvm.org/docs/ExceptionHandling.html#llvm-eh-sjlj-setjmp
 */
jmp_buf buf1;
jmp_buf buf2;
jmp_buf buf3;
jmp_buf buf4;

int g_called;

__attribute__((noinline)) void
g(jmp_buf buf, int x)
{
        g_called++;
        if (x == 0) {
                printf("just returning\n");
                return;
        }
        printf("calling longjmp\n");
        longjmp(buf, 1);
        printf("longjmp returned\n");
}

__attribute__((noinline)) void
f(jmp_buf buf, int x)
{
        int ret;
        ret = setjmp(buf4);
        printf("setjmp(buf4) returned %d\n", ret);
        if (ret != 0) {
                printf("SHOULD NOT REACH HERE!\n");
                exit(1);
        }
        printf("calling g\n");
        g(buf, x);
        printf("g returned\n");
}

int count = 5;

void
loop()
{
        int ret;
        ret = setjmp(buf2);
        printf("setjmp(buf2) returned %d\n", ret);
        if (ret == 0) {
                printf("calling f\n");
                f(buf2, 1);
                printf("f returned\n");
        }
        ret = setjmp(buf3);
        printf("setjmp(buf3) returned %d\n", ret);
        if (ret != 0) {
                printf("SHOULD NOT REACH HERE!\n");
                exit(1);
        }
        if (--count > 0) {
                printf("count %u\n", count);
                f(buf2, 1);
        }
}

int
main()
{
        int ret;
        printf("calling setjmp\n");
        ret = setjmp(buf1);
        printf("setjmp(buf1) returned %d\n", ret);
        if (ret == 0) {
                loop();
                printf("calling f\n");
                f(buf1, 0);
                printf("f returned\n");
                printf("calling f\n");
                f(buf1, 1);
                printf("f returned\n");
        }
        if (g_called != 7) {
                printf("UNEXPECTED g_called %d\n", g_called);
                exit(1);
        }
        printf("done\n");
}
