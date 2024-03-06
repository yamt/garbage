#! /bin/sh

set -e
set -x

#CC=/opt/wasi-sdk-21.0/bin/clang
WASI_SYSROOT=/opt/wasi-sdk-21.0/share/wasi-sysroot
CC="/Volumes/PortableSSD/llvm/build/bin/clang --sysroot ${WASI_SYSROOT} -resource-dir /Volumes/PortableSSD/llvm/llvm/lib/clang/17"
# binaryen with https://github.com/WebAssembly/binaryen/pull/6210
WASM_OPT=~/git/wasm/binaryen/b/bin/wasm-opt

# toywasm v36.0.0 or later with TOYWASM_ENABLE_WASM_EXCEPTION_HANDLING=ON
# and TOYWASM_ENABLE_DYLD=ON
TOYWASM=toywasm

${CC} \
-c \
--target=wasm32-wasi \
-mllvm -wasm-enable-sjlj \
-Os \
rt.c

${CC} \
-c \
--target=wasm32-wasi \
-mllvm -wasm-enable-sjlj \
-mllvm -experimental-wasm-enable-alt-sjlj \
-fPIC \
-Os \
-o a-pic.o \
a.c

${CC} \
-c \
--target=wasm32-wasi \
-mllvm -wasm-enable-sjlj \
-mllvm -experimental-wasm-enable-alt-sjlj \
-Os \
a.c

${CC} \
--target=wasm32-wasi \
-o test-sjlj.wasm \
a.o rt.o

${CC} \
--target=wasm32-wasi \
-Xlinker -pie \
-Xlinker --export-if-defined=__main_argc_argv \
-Xlinker --import-memory \
-Xlinker --export-memory \
-o test-sjlj-shared.wasm \
a-pic.o rt.o

${WASM_OPT} \
--translate-to-new-eh -all \
--strip-dwarf \
-o test-sjlj.wasm.neweh \
test-sjlj.wasm

${WASM_OPT} \
--translate-to-new-eh -all \
--strip-dwarf \
-o test-sjlj-shared.wasm.neweh \
test-sjlj-shared.wasm

${TOYWASM} --wasi test-sjlj.wasm.neweh
${TOYWASM} --dyld --dyld-path=${WASI_SYSROOT}/lib/wasm32-wasi --wasi test-sjlj-shared.wasm.neweh
