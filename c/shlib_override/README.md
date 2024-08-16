# expected behavior

inc_mem_impl in main.c overrides the one in lib2.c

# problem

the symbol overriding doesn't work with wasm dynamic-linking.

when building a shared library, wasm-ld seems to produce the code
to call the in-module version of the function directly, not via
the symbol. i consider it a bug.

```wat
% wasm2wat lib-wasi.so

....

  (func $inc_mem (type 2) (param i32)
    local.get 0
    call $inc_mem_impl)
  (func $inc_mem_impl (type 2) (param i32)
```
