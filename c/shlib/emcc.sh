#! /bin/sh

set -e
set -x

#CFLAGS=-mtail-call

emcc -sSIDE_MODULE ${CFLAGS} -o libbar.so bar.c
emcc -sSIDE_MODULE ${CFLAGS} -o libfoo.so foo.c
emcc -sSIDE_MODULE ${CFLAGS} -o libbaz.so baz.c
# -sERROR_ON_UNDEFINED_SYMBOLS=0: see https://github.com/emscripten-core/emscripten/issues/19861
# the strnlen call in baz.c doesn't work with -sMAIN_MODULE=2
# cf. https://emscripten.org/docs/compiling/Dynamic-Linking.html#code-size
emcc -v -g -sMAIN_MODULE=1 ${CFLAGS} -sERROR_ON_UNDEFINED_SYMBOLS=0 \
-o main main.c main2.c libfoo.so libbar.so

node main

#emcc -v -g -mextended-const -sMAIN_MODULE=2 -sERROR_ON_UNDEFINED_SYMBOLS=0 -o main main.c main2.c libfoo.so libbar.so
#node --experimental-wasm-extended-const main
