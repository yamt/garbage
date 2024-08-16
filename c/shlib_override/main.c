extern void inc_mem(int *p);

#if defined(__wasm__)
__attribute__((export_name("inc_mem_impl")))
#endif
void
inc_mem_impl(int *p)
{
        (*p)++;
}

int
main(int argc, char **argv)
{
        int x = 42;
        inc_mem(&x);
        if (x != 43) {
                __builtin_trap();
        }
}
