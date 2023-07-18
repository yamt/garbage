#! /bin/sh

set -e
set -x

#CFLAGS=-mtail-call

emcc -sSIDE_MODULE ${CFLAGS} -o libbar.so bar.c
emcc -sSIDE_MODULE ${CFLAGS} -o libfoo.so foo.c
# -sERROR_ON_UNDEFINED_SYMBOLS=0: see https://github.com/emscripten-core/emscripten/issues/19861
emcc -v -g -sMAIN_MODULE=2 ${CFLAGS} -sERROR_ON_UNDEFINED_SYMBOLS=0 \
-o main main.c main2.c libfoo.so libbar.so

node main

#emcc -v -g -mextended-const -sMAIN_MODULE=2 -sERROR_ON_UNDEFINED_SYMBOLS=0 -o main main.c main2.c libfoo.so libbar.so
#node --experimental-wasm-extended-const main
