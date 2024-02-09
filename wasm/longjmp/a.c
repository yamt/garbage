
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__wasm__)
/* for some reasons, __builtin_setjmp/__builtin_longjmp is not used */
typedef void *jmp_buf[2];
int setjmp(jmp_buf env) __attribute__((__returns_twice__));
void longjmp(jmp_buf env, int val);
#else
#if 1
#include <setjmp.h>
#else
/*
 * note: a jmp_buf for __builtin_setjmp is of 5 words
 * https://gcc.gnu.org/onlinedocs/gcc/Nonlocal-Gotos.html
 * https://llvm.org/docs/ExceptionHandling.html#llvm-eh-sjlj-setjmp
 */
typedef void *jmp_buf[5];
#define setjmp __builtin_setjmp
#define longjmp __builtin_longjmp
#endif
#endif

jmp_buf buf1;
jmp_buf buf2;
jmp_buf buf3;
jmp_buf buf4;

int g_called;

#define unreachable()                                                         \
        do {                                                                  \
                printf("%s %u UNREACHABLE\n", __func__, __LINE__);            \
                __builtin_trap();                                             \
        } while (0)

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
                printf("SHOULD NOT REACH HERE! (buf4)\n");
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
                printf("SHOULD NOT REACH HERE! (buf3)\n");
                exit(1);
        }
        if (--count > 0) {
                printf("count %u\n", count);
                f(buf2, 1);
        }
}

void
test1()
{
        printf("%s start\n", __func__);
        jmp_buf b1;
        jmp_buf b2;
        if (setjmp(b1)) {
                printf("%s success\n", __func__);
                return;
        }
        if (setjmp(b2)) {
                unreachable();
        }
        longjmp(b1, 1);
        unreachable();
}

void
test2()
{
        printf("%s start\n", __func__);
        jmp_buf b1;
        jmp_buf b2;
        if (setjmp(b1)) {
                unreachable();
        }
        if (setjmp(b2)) {
                printf("%s success\n", __func__);
                return;
        }
        longjmp(b2, 1);
        unreachable();
}

void
test3()
{
        printf("%s start\n", __func__);
        /*
         * C standard:
         * > All accessible objects have values, and all other components
         * > of the abstract machine have state, as of the time the
         * > longjmp function was called, except that the values of objects
         * > of automatic storage duration that are local to the function
         * > containing the invocation of the corresponding setjmp macro
         * > that do not have volatile-qualified type and have been changed
         * > between the setjmp invocation and longjmp call are indeterminate.
         */
        volatile int local = 123;
        static int staticvar = 123;
        jmp_buf b1;
        /*
         * XXX: maybe b2 should be volatile as it's "modified"
         * between setjmp(b1) and longjmp(b1, 1).
         * anyway setjmp/longjmp doesn't take volatile jmp_buf though.
         */
        jmp_buf b2;
        if (setjmp(b1)) {
                if (staticvar != 124) {
                        printf("staticvar %d != %d\n", local, 124);
                        unreachable();
                }
                if (local != 124) {
                        printf("local %d != %d\n", local, 124);
                        unreachable();
                }
                local++;
                staticvar++;
                longjmp(b2, 1);
                unreachable();
        }
        if (setjmp(b2)) {
                if (staticvar != 125) {
                        printf("staticvar %d != %d\n", local, 125);
                        unreachable();
                }
                if (local != 125) {
                        printf("local %d != %d\n", local, 125);
                        unreachable();
                }
                printf("%s success\n", __func__);
                return;
        }
        local++;
        staticvar++;
        longjmp(b1, 1);
        unreachable();
}

void
test4()
{
        printf("%s start\n", __func__);
        jmp_buf b1;
        if (setjmp(b1)) {
                printf("%s success\n", __func__);
                return;
        }
        /*
         * C standard:
         * > The longjmp function cannot cause the setjmp macro to return
         * > the value 0; if val is 0, the setjmp macro returns the value 1.
         */
        longjmp(b1, 0);
        unreachable();
}

void
test5()
{
        printf("%s start\n", __func__);
        jmp_buf b1;
        int i = setjmp(b1);
        if (i < 10) {
                longjmp(b1, i + 1);
                unreachable();
        }
        printf("%s success\n", __func__);
}

void test6_1(jmp_buf t, int x);

__attribute__((noinline)) void
test6_2(jmp_buf t, int x)
{
        printf("%s: %p x=%u called\n", __func__, t, x);
        if (x > 0) {
                jmp_buf b;
                jmp_buf b1;
                jmp_buf b2;
                if (setjmp(b1)) {
                        unreachable();
                }
                if (!setjmp(b)) {
                        if (x == 5) {
                                test6_1(b, x - 1);
                                unreachable();
                        }
                        test6_1(t, x - 1);
                        if (x >= 5) {
                                unreachable();
                        }
                }
                if (setjmp(b2)) {
                        unreachable();
                }
                if (x != 5) {
                        printf("%s: %p %u != 5\n", __func__, t, x);
                        exit(1);
                }
        }
        printf("%s: %p x=%u longjmp\n", __func__, t, x);
        if (t == NULL) {
                unreachable();
        }
        longjmp(t, 1);
        unreachable();
}

__attribute__((noinline)) void
test6_1(jmp_buf t, int x)
{
        printf("%s: %p x=%u called\n", __func__, t, x);
        jmp_buf b;
        if (!setjmp(b)) {
                test6_2(t ? t : b, x);
        }
        printf("%s: %p x=%u returning\n", __func__, t, x);
        if (x != 10) {
                unreachable();
        }
}

void
test6()
{
        test6_1(NULL, 10);
}

void
test7()
{
        jmp_buf b;
        volatile int i;
        for (i = 0; i < 10; i++) {
                if (setjmp(b)) {
                        unreachable();
                }
                if (setjmp(b)) {
                        if (i == 10) {
                                return;
                        }
                        printf("%s: %d\n", __func__, i);
                        unreachable();
                }
        }
        longjmp(b, 1);
        unreachable();
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
        test1();
        test2();
        test3();
        test4();
        test5();
        test6();
        test7();
        printf("done\n");
}
