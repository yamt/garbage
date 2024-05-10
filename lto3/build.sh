#! /bin/sh

set -e
set -x

CC=${CC:-"/opt/wasi-sdk-22.0/bin/clang -Os -flto=thin -pthread -target wasm32-wasi-threads"}

${CC} -c a.c
${CC} -c main.c
${CC} main.o a.o
