#include <assert.h>
#include <errno.h>
#include <dlfcn.h>
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

//#define USE_MAIN_VOID
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

        printf("get_foo_set_ptr() = %p\n", get_foo_set_ptr());
        printf("foo_set = %p\n", foo_set);
        printf("foo_set_p = %p\n", foo_set_p);
        assert(get_foo_set_ptr() == foo_set);

        foo_set(200);
        foo_set_p(100);
        // return foo_get();
        printf("%d (expected 100)\n", foo_get());

        printf("%d (expected 10 for flat namespace, 11 for two-level "
               "namespace)\n",
               func_in_foo(3));

        printf("%s\n", call_func_in_main());

        extern const char *get_a_value_in_foo_via_bar();
        printf("a value in foo via bar: %s\n", get_a_value_in_foo_via_bar());

        extern const char *get_a_value_in_bar_via_foo();
        printf("a value in bar via foo: %s\n", get_a_value_in_bar_via_foo());

        printf("&weak_var = %p\n", &weak_var);
        printf("weak_func = %p\n", (void *)weak_func);

        typedef const char *(*fn_t)();
        extern fn_t return_weak_func2();
        printf("return_weak_func2() = %p\n", return_weak_func2());
        assert(return_weak_func2() == NULL);

        /* test internal GOT */
        extern int var_in_main2;
        extern int *get_ptr_of_var_in_main2();

        extern const char *func_in_main2();
        extern const char *(*get_ptr_of_func_in_main2())();

        printf("var_in_main2: %u\n", var_in_main2);
        printf("&var_in_main2: %p\n", (void *)&var_in_main2);
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
        void *var = dlsym(h, "var");
        if (var == NULL) {
                printf("dlsym var failed\n");
                exit(1);
        }
        printf("var = %p\n", var);
        printf("*var = %d\n", *(int *)var);
        void *fn = dlsym(h, "fn");
        if (fn == NULL) {
                printf("dlsym fn failed\n");
                exit(1);
        }
        printf("fn = %p\n", fn);
        printf("calling fn @ baz...\n");
        int (*fn1)(const char *) = fn;
        printf("fn(\"main\") = %d (expectd 4)\n", fn1("main"));

        printf("errno = %d\n", errno);
}

__attribute__((constructor(50))) static void
ctor(void)
{
        printf("this is %s @ %s\n", __func__, __FILE__);
}
