/*
 * https://github.com/llvm/llvm-project/blob/main/llvm/lib/Target/WebAssembly/WebAssemblyLowerEmscriptenEHSjLj.cpp
 *
 * XXX if i understand it correctly, repeated setjmp calls would grow
 * the table indefinetely. am i missing something?
 */

#include <stdint.h>

struct entry {
        uint32_t id;
        uint32_t label;
};

struct state {
        uint32_t id;
        uint32_t size;
        struct arg {
                void *env;
                int val;
        } arg;
} g_state; /* XXX should be thread-local */

void *
saveSetjmp(void *env, uint32_t label, void *table, uint32_t size)
{
        struct state *state = &g_state;
        struct entry *e = table;
        uint32_t i;
        for (i = 0; i < size; i++) {
                if (e[i].id == 0) {
                        uint32_t id = ++state->id;
                        *(uint32_t *)env = id;
                        e[i].id = id;
                        e[i].label = label;
                        goto done;
                }
        }
        /* TODO grow the table */
        __builtin_trap();
done:
        state->size = size;
        return table;
}

uint32_t
testSetjmp(unsigned int id, void *table, uint32_t size)
{
        struct entry *e = table;
        uint32_t i;
        for (i = 0; i < size; i++) {
                if (e[i].id == id) {
                        return e[i].label;
                }
        }
        return 0;
}

uint32_t
getTempRet0()
{
        struct state *state = &g_state;
        return state->size;
}

void
__wasm_longjmp(void *env, int val)
{
        struct state *state = &g_state;
        struct arg *arg = &state->arg;
        arg->env = env;
        arg->val = val;
        __builtin_wasm_throw(1, arg);
}
