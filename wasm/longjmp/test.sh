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
-fPIC \
-shared -fvisibility=default \
--target=wasm32-wasi \
-mllvm -wasm-enable-sjlj \
-Os \
-o rt.so \
rt.c

${CC} \
-fPIC \
-shared -fvisibility=default \
--target=wasm32-wasi \
-mllvm -wasm-enable-sjlj \
-mllvm -experimental-wasm-enable-alt-sjlj \
-fPIC \
-Os \
-o lib.so \
lib.c rt.so

${CC} \
-c \
--target=wasm32-wasi \
-mllvm -wasm-enable-sjlj \
-mllvm -experimental-wasm-enable-alt-sjlj \
-Os \
a.c lib.c

${CC} \
--target=wasm32-wasi \
-o test-sjlj.wasm \
a.o lib.o rt.o

# note: this is a bit broken as the executable will define the __c_longjmp
# tag by itself rather than importing it.
# possible fixes:
# - build rt.o as a (part of) shared library like libc.
# - make llvm create the tag at link time (as it is for linear memory)
# cf. https://docs.google.com/document/d/1ZvTPT36K5jjiedF8MCXbEmYjULJjI723aOAks1IdLLg/edit#bookmark=id.lmq7kt1mwwoh
${CC} \
--target=wasm32-wasi \
-Xlinker -pie \
-Xlinker --export-if-defined=__main_argc_argv \
-Xlinker --import-memory \
-Xlinker --export-memory \
-o test-sjlj-shared.wasm \
a-pic.o rt.so lib.so

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

${TOYWASM} --wasi test-sjlj.wasm.neweh

${TOYWASM} --dyld \
--dyld-path=${WASI_SYSROOT}/lib/wasm32-wasi \
--dyld-path=. \
--wasi test-sjlj-shared.wasm.neweh
