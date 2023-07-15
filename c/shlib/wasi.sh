#! /bin/sh

set -e
set -x

# https://github.com/yamt/wasi-libc/tree/dynamic-linking-__main_void.o-revert 
# also, i copied libc.so to the current directory.
WASI_SDK=${WASI_SDK:-/Volumes/PortableSSD/git/component-linking-demo/wasi-sdk/build/install/opt/wasi-sdk}

# https://github.com/yamt/toywasm/pull/74
TOYWASM=${TOYWASM:-/Users/yamamoto/git/toywasm/b/toywasm}

CC=${WASI_SDK}/bin/clang
CFLAGS="-O3 -fPIC"
CLINKFLAGS="-Xlinker --experimental-pic"

${CC} ${CFLAGS} ${CLINKFLAGS} -shared -fvisibility=default -o libbar.so bar.c
${CC} ${CFLAGS} ${CLINKFLAGS} -shared -fvisibility=default -o libfoo.so foo.c
${CC} ${CFLAGS} ${CLINKFLAGS} \
-Xlinker -pie \
-Xlinker --export-if-defined=__main_argc_argv \
-Xlinker --import-memory \
-o main main.c libfoo.so libbar.so

${TOYWASM} --wasi --dyld main
