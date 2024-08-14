void *
f(int x)
{
        extern int __heap_base;
        extern int __heap_end;
        switch (x) {
        case 0:
                return &__heap_base;
        case 1:
                return &__heap_end;
        case 99:
                __builtin_wasm_throw(0, 0); /* __cpp_exception */
        default:
                __builtin_wasm_throw(1, 0); /* __c_longjmp */
        }
}
