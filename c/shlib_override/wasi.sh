#! /bin/sh

set -e
set -x

CC=/opt/wasi-sdk-24.0/bin/clang
AR=/opt/wasi-sdk-24.0/bin/ar
TOYWASM=${TOYWASM:-toywasm}

# static build
${CC} -Os -c -o lib1.o lib1.c
${CC} -Os -c -o lib2.o lib2.c
rm -f lib.a
${AR} crs lib.a lib1.o lib2.o
${CC} -Os -o wasi.static main.c lib.a

# pie dynamic-linking build
${CC} -Os -fPIC -shared -fvisibility=default -o lib.so lib1.c lib2.c
${CC} -Os -fPIC -fvisibility=default -Xlinker -pie -Xlinker --import-memory -Xlinker --export-if-defined=__main_argc_argv -o wasi.pie main.c lib.so

# this works
${TOYWASM} --wasi wasi.static

# this doesn't work
LIBC_SO_DIR=$(dirname $(${CC} --print-file-name=libc.so))
${TOYWASM} --wasi --dyld --dyld-path ${LIBC_SO_DIR} --dyld-path . wasi.pie
