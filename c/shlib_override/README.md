# expected behavior

inc_mem_impl in main.c overrides the one in lib2.c

# problem

the symbol overriding doesn't work with wasm dynamic-linking.

```shell
+ toywasm --wasi --dyld --dyld-path /Users/yamamoto/wasm/wasi-sdk-24.0-x86_64-macos/bin/../share/wasi-sysroot/lib/wasm32-wasi --dyld-path . wasi.pie
an unexpected version of inc_mem_impl was called
Error: [trap] unreachable executed (4): unreachable at 00b7ec
current pc 00b7ec
frame[  5] funcpc 00b7ec (libc.so:abort) callerpc 0001b4
frame[  4] funcpc 00016e (lib-wasi.so:inc_mem_impl) callerpc 000169
  param [0] = 00036e3c
  local [1] = 00036e20
  local [2] = 00000004
frame[  3] funcpc 000161 (lib-wasi.so:inc_mem) callerpc 0001ec
  param [0] = 00036e3c
frame[  2] funcpc 0001c9 (wasi.pie:main) callerpc 00b0f2
  param [0] = 00000001
  param [1] = 00036e70
  local [2] = 00036e30
frame[  1] funcpc 00b080 (libc.so:__main_void) callerpc 00019a
  local [0] = 00036e40
  local [1] = 00036e70
  local [2] = 00036e60
frame[  0] funcpc 000166 (wasi.pie:_start)
  local [0] = 00000000
```

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
