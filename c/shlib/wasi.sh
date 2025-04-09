#! /bin/sh

set -e
set -x

WASI_SDK=${WASI_SDK:-/opt/wasi-sdk-24.0}
CC=${WASI_SDK}/bin/clang
WASI_SYSROOT=${WASI_SDK}/share/wasi-sysroot

# https://github.com/yamt/wasi-libc/tree/dynamic-linking-__main_void.o-revert 
#WASI_SDK=${WASI_SDK:-/Volumes/PortableSSD/git/component-linking-demo/wasi-sdk/build/install/opt/wasi-sdk}
#CC=${WASI_SDK}/bin/clang

#WASI_SYSROOT=${WASI_SDK}/share/wasi-sysroot
#WASI_SYSROOT=/Users/yamamoto/git/wasi-libc/sysroot

#LLVM_HOME=/Volumes/PortableSSD/llvm/llvm
#LLVM_HOME=/Volumes/PortableSSD/llvm/build
#LLVM_HOME=/Volumes/PortableSSD/git/component-linking-demo/wasi-sdk/build/install/opt/wasi-sdk
#RESOURCE_DIR=/Volumes/PortableSSD/llvm/llvm/lib/clang/17
#RESOURCE_DIR=${WASI_SDK}/lib/clang/17
#CC=${LLVM_HOME}/bin/clang
#CFLAGS="${CFLAGS} --sysroot ${WASI_SYSROOT}"
#CFLAGS="${CFLAGS} -resource-dir ${RESOURCE_DIR}"

# built with
#  TOYWASM_ENABLE_DYLD=ON
#  TOYWASM_ENABLE_DYLD_DLFCN=ON
TOYWASM=${TOYWASM:-toywasm}

CFLAGS="${CFLAGS} -Os"
#CFLAGS="${CFLAGS} -I./libdl"

# https://reviews.llvm.org/D155542
#CFLAGS="${CFLAGS} -mextended-const"

# CFLAGS="${CFLAGS} -mtail-call"

CLINKFLAGS="-Xlinker --experimental-pic"
#CLIBLINKFLAGS="-shared -fvisibility=default -mexec-model=reactor"
CLIBLINKFLAGS="-shared -fvisibility=default"

#CRT1=$(${CC} --print-file-name crt1-reactor.o)

CPICFLAGS="${CFLAGS} -fPIC"

${CC} ${CPICFLAGS} ${CLINKFLAGS} ${CLIBLINKFLAGS} -o libbar.so bar.c
# see the comment in native.sh
${CC} ${CPICFLAGS} ${CLINKFLAGS} ${CLIBLINKFLAGS} -o libfoo.so foo.c libbar.so
${CC} ${CPICFLAGS} ${CLINKFLAGS} ${CLIBLINKFLAGS} -o libbaz.so baz.c

# PIE build
# Note: --import-memory is to follow the dynamic-linking convention.
# https://github.com/WebAssembly/tool-conventions/blob/main/DynamicLinking.md#interface-and-usage
# while it isn't very clear what to do for the main executable, this is what
# emscripten seems to do. (and toywasm followed it for the pie case.)
# Note: --export-memory is a wasi requirement
# Note: maybe we should --export-table as well, but the current wasm-ld
# reject the combination of -shared --export-table.
${CC} ${CPICFLAGS} ${CLINKFLAGS} \
-Xlinker -pie \
-Xlinker --export-if-defined=__main_argc_argv \
-Xlinker --import-memory \
-Xlinker --export-memory \
-o main.wasi.pie \
main.c \
main2.c \
libfoo.so libbar.so \
-ldl


# non-PIE build
# clang doesn't have DynamicNoPIC for non-darwin targets. just use -fPIC.
#PIC=-mdynamic-no-pic
PIC=-fPIC
# Note: wasm-ld doesn't find libdl.so for -ldl unless -shared or -pie is used
# https://github.com/llvm/llvm-project/blob/b01adc6bed7e5b924dd8a097be0aa893f4823905/lld/wasm/Driver.cpp#L309-L313
${CC} -v ${CFLAGS} ${CLINKFLAGS} \
${PIC} \
-nodefaultlibs \
-Xlinker --export-if-defined=__main_argc_argv \
-Xlinker --unresolved-symbols=import-dynamic \
-Xlinker --export-table \
-Xlinker --growable-table \
-Xlinker --export=__stack_pointer \
-Xlinker --export=__heap_base \
-Xlinker --export=__heap_end \
-z stack-size=16384 \
-o main.wasi.non-pie \
main.c \
main2.c \
libfoo.so libbar.so \
${WASI_SYSROOT}/lib/wasm32-wasi/libdl.so

# note: specify --dyld-path for toywasm libdl.so before the one for wasi-libc
# so that dyld picks up the former.
${TOYWASM} --wasi \
--dyld \
--dyld-dlfcn \
--dyld-path . \
--dyld-path ./libdl \
--dyld-path ${WASI_SYSROOT}/lib/wasm32-wasi \
"$@" main.wasi.non-pie

${TOYWASM} --wasi \
--dyld \
--dyld-dlfcn \
--dyld-path . \
--dyld-path ./libdl \
--dyld-path ${WASI_SYSROOT}/lib/wasm32-wasi \
"$@" main.wasi.pie
