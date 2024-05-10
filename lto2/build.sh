#! /bin/sh

set -e
set -x

CC=${CC:-"/opt/wasi-sdk-22.0/bin/clang -Os -flto=thin"}

${CC} -c a.c
${CC} -c main.c
${CC} main.o a.o
