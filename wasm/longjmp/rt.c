/*
 * https://github.com/llvm/llvm-project/blob/main/llvm/lib/Target/WebAssembly/WebAssemblyLowerEmscriptenEHSjLj.cpp
 */

#include <stddef.h>
#include <stdint.h>

/*
 * jmp_buf should have large enough size and alignment to contain
 * this structure.
 */
struct jmp_buf_impl {
        void *func_invocation_id;
        uint32_t label;

        /*
         * ideally, this can be replaced with multivalue.
         */
        struct arg {
                void *env;
                int val;
        } arg;
};

void
__wasm_sjlj_setjmp(void *env, uint32_t label, void *table)
{
        struct jmp_buf_impl *jb = env;
        if (label == 0) { /* ABI contract */
                __builtin_trap();
        }
        if (table == NULL) { /* sanity check */
                __builtin_trap();
        }
        jb->func_invocation_id = table;
        jb->label = label;
}

uint32_t
__wasm_sjlj_test(void *env, void *table)
{
        struct jmp_buf_impl *jb = env;
        if (jb->label == 0) { /* ABI contract */
                __builtin_trap();
        }
        if (table == NULL) { /* sanity check */
                __builtin_trap();
        }
        if (jb->func_invocation_id == table) {
                return jb->label;
        }
        return 0;
}

void
__wasm_sjlj_longjmp(void *env, int val)
{
        struct jmp_buf_impl *jb = env;
        struct arg *arg = &jb->arg;
        /*
         * C standard:
         * > The longjmp function cannot cause the setjmp macro to return
         * > the value 0; if val is 0, the setjmp macro returns the value 1.
         */
        if (val == 0) {
                val = 1;
        }
        arg->env = env;
        arg->val = val;
        __builtin_wasm_throw(1, arg); /* 1 == C_LONGJMP */
}
