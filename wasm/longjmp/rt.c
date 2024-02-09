/*
 * https://github.com/llvm/llvm-project/blob/main/llvm/lib/Target/WebAssembly/WebAssemblyLowerEmscriptenEHSjLj.cpp
 */

#include <stdint.h>
#include <stdlib.h>

struct jmp_buf_impl {
        void *func_invocation_id;
        uint32_t label;
};

static _Thread_local struct state {
        struct arg {
                void *env;
                int val;
        } arg;
} g_state;

void *
saveSetjmp(void *env, uint32_t label, void *table, uint32_t size)
{
        struct jmp_buf_impl *jb = env;
        if (label == 0) { /* ABI contract */
                __builtin_trap();
        }
        jb->func_invocation_id = table;
        jb->label = label;
        return table;
}

uint32_t
testSetjmp(void *env, void *table, uint32_t size)
{
        struct jmp_buf_impl *jb = env;
        if (jb->label == 0) { /* ABI contract */
                __builtin_trap();
        }
        if (jb->func_invocation_id == table) {
                return jb->label;
        }
        return 0;
}

uint32_t
getTempRet0()
{
        return 0;
}

void
__wasm_longjmp(void *env, int val)
{
        struct state *state = &g_state;
        struct arg *arg = &state->arg;
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
