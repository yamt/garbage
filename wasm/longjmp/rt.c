/*
 * https://github.com/llvm/llvm-project/blob/main/llvm/lib/Target/WebAssembly/WebAssemblyLowerEmscriptenEHSjLj.cpp
 */

struct {
        void *env;
        int val;
} arg; /* XXX should use a thread-local? */

int g_label; /* XXX single setjmp call-site is assumed for now */

void *
saveSetjmp(void *env, int label, void *table, int size)
{
        g_label = label;
        return table;
}

int
testSetjmp(int id, void *table, int size)
{
        return g_label;
}

int
getTempRet0()
{
        return 0; /* XXX should use a thread-local? */
}

void
__wasm_longjmp(void *env, int val)
{
        arg.env = env;
        arg.val = val;
        __builtin_wasm_throw(1, &arg);
}
