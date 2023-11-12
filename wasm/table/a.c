// https://reviews.llvm.org/D139010

// - a table should be static
// - export/import attributes (eg. __export_name__) can be only applied to
//   functions. (thus not to a table)

// funcref is broken?
// https://github.com/llvm/llvm-project/issues/70620

static __externref_t table1[0];
static __externref_t table2[0];
typedef void (*__funcref funcref_t)();
static __funcref table3[0];

void
set_extern_to_table1(unsigned int idx, __externref_t v)
{
        __builtin_wasm_table_set(table1, idx, v);
}

void
set_extern_to_table2(unsigned int idx, __externref_t v)
{
        __builtin_wasm_table_set(table2, idx, v);
}

void
set_extern_to_table3(unsigned int idx, funcref_t v)
{
        __builtin_wasm_table_grow(table3, v, 1);
        //__builtin_wasm_table_set(table3, idx, v);
}
