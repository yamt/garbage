#! /bin/sh

set -e
set -x

#CFLAGS=-mtail-call

emcc -sSIDE_MODULE ${CFLAGS} -o libbar.so bar.c

# XXX: if we link libbar.so this way, for some reasons i don't understand,
# emscripten aborts like the following:
#
# + node main
# Aborted(Assertion failed: undefined symbol 'weak_func2'. perhaps a side module was not linked in? if this global was expected to arrive from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment)
# /Users/yamamoto/git/garbage/c/shlib/main:129
#       throw ex;
#       ^
#
# RuntimeError: Aborted(Assertion failed: undefined symbol 'weak_func2'. perhaps a side module was not linked in? if this global was expected to arrive from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment)
#     at abort (/Users/yamamoto/git/garbage/c/shlib/main:701:11)
#     at assert (/Users/yamamoto/git/garbage/c/shlib/main:368:5)
#     at reportUndefinedSymbols (/Users/yamamoto/git/garbage/c/shlib/main:2212:11)
#     at /Users/yamamoto/git/garbage/c/shlib/main:2238:9
#
# emcc -sSIDE_MODULE ${CFLAGS} -o libfoo.so foo.c libbar.so

emcc -sSIDE_MODULE ${CFLAGS} -o libfoo.so foo.c
emcc -sSIDE_MODULE ${CFLAGS} -o libbaz.so baz.c
# -sERROR_ON_UNDEFINED_SYMBOLS=0: see https://github.com/emscripten-core/emscripten/issues/19861
# the strnlen call in baz.c doesn't work with -sMAIN_MODULE=2
# cf. https://emscripten.org/docs/compiling/Dynamic-Linking.html#code-size
emcc -g -sMAIN_MODULE=1 ${CFLAGS} -sERROR_ON_UNDEFINED_SYMBOLS=0 \
-o main main.c main2.c -L. libfoo.so libbar.so

node main

#emcc -v -g -mextended-const -sMAIN_MODULE=2 -sERROR_ON_UNDEFINED_SYMBOLS=0 -o main main.c main2.c libfoo.so libbar.so
#node --experimental-wasm-extended-const main
