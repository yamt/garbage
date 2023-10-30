// https://reviews.llvm.org/D139010

static __externref_t table[0];

void
set_func(unsigned int idx, void *fn)
{
        __builtin_wasm_table_set(table, idx, fn);
}
