#! /bin/sh

../../listenexec/listenexec -p 9999 ~/git/toywasm/b/toywasm --wasi --wasi-dir=. httpd.wasm

# eg.
# ab -c5 -n10000 "http://[::1]:9999/Makefile"
