#! /bin/sh

set -e

WASI_SDK=${WASI_SDK:-/opt/wasi-sdk-21.0}
#CC="${WASI_SDK}/bin/clang --sysroot ${HOME}/git/wasi-libc/sysroot"
CC="/Volumes/PortableSSD/llvm/build/bin/clang --sysroot ${HOME}/git/wasi-libc/sysroot -resource-dir /Users/yamamoto/wasm/wasi-sdk-24.0-x86_64-macos/lib/clang/18"
AR=${WASI_SDK}/bin/ar

./clean.sh

${CC} -c -o main.o main.c
${AR} crs libmain.a main.o

${CC} -c -o foo.o foo.c

${CC} -v foo.o -L. -lmain
#-Xlinker -no-gc-sections #$(${CC} --print-file-name crt2-command.o)
