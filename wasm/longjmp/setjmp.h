#if defined(__wasm__)
#if defined(__EMSCRIPTEN__)
#include <setjmp.h>
#else
/* for some reasons, __builtin_setjmp/__builtin_longjmp is not used */
typedef void *jmp_buf[4];
int setjmp(jmp_buf env) __attribute__((__returns_twice__));
void longjmp(jmp_buf env, int val);
#endif
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
