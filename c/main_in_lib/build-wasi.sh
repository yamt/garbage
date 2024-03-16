#! /bin/sh

set -e

WASI_SDK=${WASI_SDK:-/opt/wasi-sdk-21.0}
CC=${WASI_SDK}/bin/clang
AR=${WASI_SDK}/bin/ar

./clean.sh

${CC} -c -o main.o main.c
${AR} crs libmain.a main.o

${CC} -c -o foo.o foo.c

${CC} foo.o -L. -lmain
