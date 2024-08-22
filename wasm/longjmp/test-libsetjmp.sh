#! /bin/sh

set -e

##CC=/opt/wasi-sdk-21.0/bin/clang
CC="/Volumes/PortableSSD/llvm/build/bin/clang --sysroot /opt/wasi-sdk-24.0/share/wasi-sysroot -resource-dir /Volumes/PortableSSD/llvm/llvm/lib/clang/17"
# binaryen with https://github.com/WebAssembly/binaryen/pull/6210
WASM_OPT=~/git/wasm/binaryen/b/bin/wasm-opt
# toywasm v36.0.0 or later with TOYWASM_ENABLE_WASM_EXCEPTION_HANDLING=ON
TOYWASM=toywasm

${CC} \
--target=wasm32-wasi \
-mllvm -wasm-enable-sjlj \
-DUSE_SETJMP_H \
-Os \
a.c -lsetjmp

${WASM_OPT} \
--translate-to-exnref -all \
--strip-dwarf \
-o a.out.neweh \
a.out

${TOYWASM} --wasi a.out.neweh
