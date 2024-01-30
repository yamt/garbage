#! /bin/sh

CC=/opt/wasi-sdk-21.0/bin/clang
# binaryen with https://github.com/WebAssembly/binaryen/pull/6210
WASM_OPT=~/git/wasm/binaryen/b/bin/wasm-opt
# toywasm v36.0.0 or later with TOYWASM_ENABLE_WASM_EXCEPTION_HANDLING=ON
TOYWASM=toywasm

${CC} \
--target=wasm32-wasi \
-mllvm -wasm-enable-sjlj \
a.c rt.c

${WASM_OPT} \
--translate-eh-old-to-new -all \
--strip-dwarf \
-o a.out.neweh \
a.out

${TOYWASM} --wasi a.out.neweh
