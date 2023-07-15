#include <assert.h>
#include <stdio.h>
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

const char *call_func_in_main();

__attribute__((weak)) extern int weak_var;
__attribute__((weak)) extern int weak_func();

#if 0
int
main()
#else
int
main(int argc, char **argv)
#endif
{
#if 1
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
        // weak_func();
}
