#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void foo_set(int x);
void (*get_foo_set_ptr())(int);
int foo_get();
int func_in_foo(int n);

void (*foo_set_p)(int) = foo_set;

extern char *optarg;

/* try to override a function in libbar */
#if defined(__wasi__)
__attribute__((export_name("func_in_bar")))
#endif
int
func_in_bar(int n)
{
        printf("%s in %s called\n", __func__, __FILE__);
        return n + 4;
}

#if defined(__wasi__)
__attribute__((export_name("func_in_main")))
#endif
const char *
func_in_main()
{
        return "func in main";
}

extern int recurse_bar(int i);

#if defined(__wasi__)
__attribute__((export_name("recurse_main")))
#endif
int
recurse_main(int i)
{
        if (i < 0) {
                printf("%s %d fp=%p\n", __func__, i,
                       (void *)__builtin_frame_address(0));
                return i;
        }
#if defined(__has_attribute)
#if __has_attribute(musttail)
#if !defined(__wasm__) || defined(__wasm_tail_call__)
        __attribute__((musttail))
#endif
#endif
#endif
        return recurse_bar(i);
}

const char *call_func_in_main();

__attribute__((weak)) extern int weak_var;
__attribute__((weak)) extern int weak_func();

/* note: this export should not interfere dlopen/dlsym */
#if defined(__wasi__)
__attribute__((export_name("fn")))
#endif
int
fn(const char *caller)
{
        printf("hi, %s. this is %s @ %s\n", caller, __func__, __FILE__);
        abort();
}

// #define USE_MAIN_VOID
#undef USE_MAIN_VOID

#if defined(USE_MAIN_VOID)
int
main()
#else
int
main(int argc, char **argv)
#endif
{
#if !defined(USE_MAIN_VOID)
        int ch;
        while ((ch = getopt(argc, argv, "f:")) != -1) {
                switch (ch) {
                case 'f':
                        printf("f: %s\n", optarg);
                        break;
                }
        }
#endif

        printf("calling get_foo_set_ptr()...\n");
        printf("get_foo_set_ptr() = %p\n", get_foo_set_ptr());
        printf("foo_set = %p\n", foo_set);
        printf("foo_set_p = %p\n", foo_set_p);
        printf("calling get_foo_set_ptr()...\n");
        assert(get_foo_set_ptr() == foo_set);

        printf("foo_set = %p\n", foo_set);
        printf("foo_set_p = %p\n", foo_set_p);
        assert(foo_set == foo_set_p);
        printf("calling foo_set()...\n");
        foo_set(200);
        printf("calling foo_set_p()...\n");
        foo_set_p(100);
        // return foo_get();
        printf("foo_get foo_set_p()...\n");
        printf("%d (expected 100)\n", foo_get());
        assert(foo_get() == 100);

        printf("%d (expected 10 for flat namespace, 11 for two-level "
               "namespace)\n",
               func_in_foo(3));
#if defined(__wasi__)
        assert(func_in_foo(3) == 10);
#endif

        printf("%s\n", call_func_in_main());

        extern const char *get_a_value_in_foo_via_bar();
        printf("a value in foo via bar: %s\n", get_a_value_in_foo_via_bar());
        assert(!strcmp(get_a_value_in_foo_via_bar(),
                       "this is a value in foo"));

        extern const char *get_a_value_in_bar_via_foo();
        printf("a value in bar via foo: %s\n", get_a_value_in_bar_via_foo());
        assert(!strcmp(get_a_value_in_bar_via_foo(),
                       "this is a value in bar"));

        printf("&weak_var = %p\n", &weak_var);
        printf("weak_func = %p\n", (void *)weak_func);
        assert(&weak_var == NULL);
        assert(weak_func == NULL);

        typedef const char *(*fn_t)();
        extern fn_t return_weak_func2();
        printf("calling return_weak_func2()...\n");
        printf("return_weak_func2() = %p\n", return_weak_func2());
        assert(return_weak_func2() == NULL);

        /* test internal GOT */
        extern int var_in_main2;
        extern int *get_ptr_of_var_in_main2();

        extern const char *func_in_main2();
        extern const char *(*get_ptr_of_func_in_main2())();

        printf("var_in_main2: %u\n", var_in_main2);
        assert(var_in_main2 == 321);
        printf("&var_in_main2: %p\n", (void *)&var_in_main2);
        printf("calling get_ptr_of_var_in_main2()...\n");
        printf("get_ptr_of_var_in_main2(): %p\n",
               (void *)get_ptr_of_var_in_main2());
        assert(get_ptr_of_var_in_main2() == &var_in_main2);
        assert(get_ptr_of_var_in_main2() != NULL);

        printf("func_in_main2: %s\n", func_in_main2());
        printf("&func_in_main2: %p\n", (void *)func_in_main2);
        printf("get_ptr_of_func_in_main2(): %p\n",
               (void *)get_ptr_of_func_in_main2());
        assert(get_ptr_of_func_in_main2() == func_in_main2);
        assert(get_ptr_of_func_in_main2() != NULL);

        /* note: libfoo is loaded earlier than libbar */
        extern const char *var_to_override;
        extern const char *func_to_override();
        printf("var_to_override: %s (expected foo)\n", var_to_override);
        printf("func_to_override: %s (expected foo)\n", func_to_override());
        assert(!strcmp(var_to_override, "var_to_override foo"));
        assert(!strcmp(func_to_override(), "func_to_override foo"));

        /* test tail-call between instances */
        int n = 100;
#if defined(__has_attribute)
#if __has_attribute(musttail)
#if !defined(__wasm__) || defined(__wasm_tail_call__)
        /* assume tail-call */
        n = 1000000;
#endif
#endif
#endif
        printf("calling recurse_bar(%d) fp=%p\n", n,
               (void *)__builtin_frame_address(0));
        recurse_bar(n);

        /* dlopen */
        printf("dlopen'ing libbaz.so...\n");
        void *h = dlopen("libbaz.so", RTLD_LAZY);
        if (h == NULL) {
                printf("dlopen failed %s\n", dlerror());
                exit(1);
        }

        printf("errno = %d\n", errno);
        assert(errno == 4321); /* set by baz ctor */

        void *var = dlsym(h, "var");
        if (var == NULL) {
                printf("dlsym var failed\n");
                exit(1);
        }
        printf("var = %p\n", var);
        printf("*var = %d\n", *(int *)var);
        assert(*(int *)var == 42);

        void *fn = dlsym(h, "fn");
        if (fn == NULL) {
                printf("dlsym fn failed\n");
                exit(1);
        }
        printf("fn = %p\n", fn);
        printf("calling fn @ baz...\n");
        int (*fn1)(const char *) = fn;
        printf("fn(\"main\") = %d (expectd 4)\n", fn1("main"));
        assert(fn1("main") == 4);

        void *(*get_printf_ptr)() = dlsym(h, "get_printf_ptr");
        if (get_printf_ptr == NULL) {
                printf("dlsym get_printf_ptr failed\n");
                exit(1);
        }
        printf("printf in main: %p\n", printf);
        printf("printf in baz: %p\n", get_printf_ptr());
        assert(printf == get_printf_ptr());

        /*
         * weak to work around:
         * https://github.com/llvm/llvm-project/issues/103592
         */
        __attribute__((weak)) extern char __heap_base;
        __attribute__((weak)) extern char __heap_end;
        __attribute__((weak)) extern char __stack_low;
        __attribute__((weak)) extern char __stack_high;
        extern uint32_t __stack_pointer __attribute__((address_space(1)));
        printf("__heap_base %p\n", &__heap_base);
        printf("__heap_end %p\n", &__heap_end);
        printf("__stack_low %p\n", &__stack_low);
        printf("__stack_high %p\n", &__stack_high);
        assert(&__heap_base != NULL);
        assert(&__heap_end != NULL);
        assert(&__stack_low != NULL);
        assert(&__stack_high != NULL);
        assert((char *)__stack_pointer != NULL);
        assert(&__heap_base < &__heap_end);
        assert(&__stack_low < &__stack_high);
        assert((&__heap_base < &__stack_low) == (&__heap_end < &__stack_high));
        assert(&__stack_low <= (char *)__stack_pointer);
        assert((char *)__stack_pointer <= &__stack_high);

        extern char *bar_heap_base(void);
        extern char *bar_heap_end(void);
        extern char *bar_stack_low(void);
        extern char *bar_stack_high(void);
        assert(&__heap_base == bar_heap_base());
        assert(&__heap_end == bar_heap_end());
        assert(&__stack_low == bar_stack_low());
        assert(&__stack_high == bar_stack_high());
}

__attribute__((constructor(50))) static void
ctor(void)
{
        printf("this is %s @ %s\n", __func__, __FILE__);
}
