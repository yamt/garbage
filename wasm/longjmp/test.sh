#! /bin/sh

set -e
set -x

#CC=/opt/wasi-sdk-21.0/bin/clang
WASI_SYSROOT=/opt/wasi-sdk-21.0/share/wasi-sysroot
# clang with https://github.com/llvm/llvm-project/pull/84137
CC="/Volumes/PortableSSD/llvm/build/bin/clang --sysroot ${WASI_SYSROOT} -resource-dir /Volumes/PortableSSD/llvm/llvm/lib/clang/17"
# binaryen with https://github.com/WebAssembly/binaryen/pull/6210
WASM_OPT=~/git/wasm/binaryen/b/bin/wasm-opt

# toywasm v36.0.0 or later with TOYWASM_ENABLE_WASM_EXCEPTION_HANDLING=ON
# and TOYWASM_ENABLE_DYLD=ON
TOYWASM=toywasm

# common options
CC="${CC} --target=wasm32-wasi -Os"

### static

${CC} \
-c \
-mllvm -wasm-enable-sjlj \
rt.c

${CC} \
-c \
-mllvm -wasm-enable-sjlj \
a.c lib.c

${CC} \
-o test-sjlj.wasm \
a.o lib.o rt.o

### dynamic

${CC} \
-fPIC \
-shared -fvisibility=default \
-mllvm -wasm-enable-sjlj \
-o rt.so \
rt.c

${CC} \
-fPIC \
-shared -fvisibility=default \
-mllvm -wasm-enable-sjlj \
-fPIC \
-o lib.so \
lib.c rt.so

${CC} \
-c \
-mllvm -wasm-enable-sjlj \
-fPIC \
-o a-pic.o \
a.c

# dynamic link involves a few subtleties.
# cf. https://docs.google.com/document/d/1ZvTPT36K5jjiedF8MCXbEmYjULJjI723aOAks1IdLLg/edit#bookmark=id.lmq7kt1mwwoh
${CC} \
-Xlinker -pie \
-Xlinker --export-if-defined=__main_argc_argv \
-Xlinker --import-memory \
-Xlinker --export-memory \
-o test-sjlj-shared.wasm \
a-pic.o rt.so lib.so

### translate to new eh

${WASM_OPT} \
--translate-to-new-eh -all \
-g \
-o test-sjlj.wasm.neweh \
test-sjlj.wasm

${WASM_OPT} \
--translate-to-new-eh -all \
-g \
-o test-sjlj-shared.wasm.neweh \
test-sjlj-shared.wasm

# note: rt.so contains only tag import and "throw" instruction,
# which don't need the --translate-to-new-eh conversion.

${WASM_OPT} \
--translate-to-new-eh -all \
-g \
-o lib.so \
lib.so

### run

${TOYWASM} --wasi test-sjlj.wasm.neweh

${TOYWASM} --dyld \
--dyld-path=${WASI_SYSROOT}/lib/wasm32-wasi \
--dyld-path=. \
--wasi test-sjlj-shared.wasm.neweh
