#! /bin/sh

set -e
set -x

emcc -sSIDE_MODULE -o libbar.so bar.c
emcc -sSIDE_MODULE -o libfoo.so foo.c
emcc -v -sMAIN_MODULE=2 -o main main.c libfoo.so libbar.so

node main
