
#include <stdint.h>
#include <stdio.h>

#if defined(__wasm__)
/* for some reasons, __builtin_setjmp/__builtin_longjmp is not used */
typedef void *jmp_buf;
int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);
#else
typedef void *jmp_buf;
#define setjmp __builtin_setjmp
#define longjmp __builtin_longjmp
#endif

/*
 * https://gcc.gnu.org/onlinedocs/gcc/Nonlocal-Gotos.html
 * https://llvm.org/docs/ExceptionHandling.html#llvm-eh-sjlj-setjmp
 */
void *buf[5];

__attribute__((noinline)) void
g(jmp_buf buf, int x)
{
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
        printf("calling g\n");
        g(buf, x);
        printf("g returned\n");
}

int
main()
{
        printf("calling setjmp\n");
        int ret = setjmp(buf);
        printf("setjmp returned %d\n", ret);
        if (ret == 0) {
                printf("calling f\n");
                f(buf, 0);
                printf("f returned\n");
                printf("calling f\n");
                f(buf, 1);
                printf("f returned\n");
        }
        printf("done\n");
}
