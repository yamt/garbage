#! /bin/sh

set -e

emcc \
-sSUPPORT_LONGJMP=wasm \
-Os \
a.c lib.c

node a.out.js

emcc -sSIDE_MODULE -sSUPPORT_LONGJMP=wasm -Os \
-o rt.so rt.c
emcc -sSIDE_MODULE -sSUPPORT_LONGJMP=wasm -Os \
-o lib.so lib.c

emcc -sMAIN_MODULE=1 -sSUPPORT_LONGJMP=wasm -Os \
-sERROR_ON_UNDEFINED_SYMBOLS=0 \
a.c -L. lib.so rt.so

node a.out.js
