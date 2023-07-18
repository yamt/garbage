#! /bin/sh

set -e
set -x

# https://github.com/yamt/wasi-libc/tree/dynamic-linking-__main_void.o-revert 
# also, i copied libc.so to the current directory.
WASI_SDK=${WASI_SDK:-/Volumes/PortableSSD/git/component-linking-demo/wasi-sdk/build/install/opt/wasi-sdk}
CC=${WASI_SDK}/bin/clang

#WASI_SYSROOT=${WASI_SDK}/share/wasi-sysroot
#LLVM_HOME=/Volumes/PortableSSD/llvm/llvm
#CC=${LLVM_HOME}/bin/clang
#CFLAGS="${CFLAGS} --sysroot ${WASI_SYSROOT}"

# https://github.com/yamt/toywasm/pull/74
# built with TOYWASM_ENABLE_DYLD=ON
TOYWASM=${TOYWASM:-/Users/yamamoto/git/toywasm/b/toywasm}

CFLAGS="${CFLAGS} -O3 -fPIC"

# https://reviews.llvm.org/D155542
# CFLAGS="${CFLAGS} -mextended-const"

# CFLAGS="${CFLAGS} -mtail-call"

CLINKFLAGS="-Xlinker --experimental-pic"

${CC} ${CFLAGS} ${CLINKFLAGS} -shared -fvisibility=default -o libbar.so bar.c
${CC} ${CFLAGS} ${CLINKFLAGS} -shared -fvisibility=default -o libfoo.so foo.c
${CC} ${CFLAGS} ${CLINKFLAGS} \
-Xlinker -pie \
-Xlinker --export-if-defined=__main_argc_argv \
-Xlinker --import-memory \
-o main \
main.c \
main2.c \
libfoo.so libbar.so

${TOYWASM} --wasi --dyld "$@" main
